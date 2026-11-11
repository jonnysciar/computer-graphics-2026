#pragma once

#include "CameraController.hpp"
#include "uniform_buffers.hpp"

#include <functional>
#include <stdexcept>
#include <vector>

struct ShadowMapperParams {
    uint32_t resolution = 64; // size of each cube face
    uint32_t lightCount = NUM_LIGHTS;   // number of shadow-casting point lights (1 .. NUM_LIGHTS)
    float nearPlane = CameraController::nearPlane;
    float farPlane = CameraController::farPlane; // must cover everything the lights reach
    std::string vertShader = "shaders/shadow.vert.spv";
    std::string fragShader = "shaders/shadow.frag.spv";
};

// Omnidirectional shadows for several point lights.
//
// Storage: ONE depth image (2D array) with 6 layers per light (layer = light * 6 + face, face
// order +X -X +Y -Y +Z -Z). The scene shader samples it as `sampler2DArrayShadow`: the GPU does
// the depth comparison and the bilinear PCF, and the shader selects the cube face itself, so no
// cube-array / dynamic sampler-array device features are needed.
// Each face is rendered with a slightly wider field of view (a guard band of GuardTexels texels)
// so that PCF taps near a face edge still read valid data.
class ShadowMapper {
  public:
    // Keep in sync with SHADOW_GUARD_TEXELS in scene.frag
    static constexpr float GuardTexels = 4.0f;

    // Return false to exclude an instance from casting shadows
    // (e.g. the lamp mesh that surrounds a light).
    using CasterFilter = std::function<bool(const Instance &)>;

    ShadowMapper() = default;

    void init(BaseProject *bp, VertexDescriptor *vd, VkDevice dev, VkPhysicalDevice physicalDev,
              ShadowMapperParams p = {}) {
        BP = bp;
        device = dev;
        physDevice = physicalDev;
        VD = vd;
        prm = std::move(p);

        if (prm.lightCount < 1 || prm.lightCount > NUM_LIGHTS)
            throw std::runtime_error("ShadowMapper: lightCount must be in [1, NUM_LIGHTS]");
        layerCount = prm.lightCount * 6;

        depthFormat = pickDepthFormat(); // also sets linearCompare
        createShadowImages();
        createSampler();
        createRenderPass(); // Starter RenderPass: init only, create() is in pipelinesAndDescriptorSetsInit

        DSLshadow.init(BP, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,
                             static_cast<int>(sizeof(ShadowUBO)), 1}});

        VkPushConstantRange pk{};
        pk.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pk.offset = 0;
        pk.size = sizeof(uint32_t); // layer = light * 6 + face

        pipeline.init(BP, VD, prm.vertShader, prm.fragShader, {&DSLshadow}, {pk});
        pipeline.setCullMode(VK_CULL_MODE_NONE); // robust against the cube-map winding flip
    }

    // Call in localInit(), AFTER scene.init(...) (and after you set DPSZs).
    // Collects the casters and reserves descriptor pool space for them.
    // Only instances whose model uses the same VertexDescriptor given to init()
    // are considered (the shadow pipeline has a single vertex layout).
    void attachScene(Scene *sc, CasterFilter filter = nullptr) {
        scene = sc;
        casters.clear();
        for (int k = 0; k < scene->TechniqueInstanceCount; k++) {
            for (int j = 0; j < scene->TI[k].InstanceCount; j++) {
                Instance &in = scene->TI[k].I[j];
                Model *m = scene->M[in.Mid];
                if (m->getVD() != VD)
                    continue;
                if (filter && !filter(in))
                    continue;
                casters.emplace_back();
                casters.back().inst = &in;
                casters.back().model = m;
            }
        }
        BP->DPSZs.uniformBlocksInPool += (int)casters.size();
        BP->DPSZs.setsInPool += (int)casters.size();
        std::cout << "ShadowMapper: " << casters.size() << " shadow casters, " << prm.lightCount << " lights\n";
    }

    // ------------------------------------- pipelinesAndDescriptorSetsInit
    void pipelinesAndDescriptorSetsInit() {
        renderPass.create();        // VkRenderPass (+ placeholder image and framebuffers, replaced below)
        bindLayerFramebuffers();    // one framebuffer per shadow layer
        pipeline.create(&renderPass);
        for (auto &c : casters)
            c.ds.init(BP, &DSLshadow, {});
    }

    // ------------------------------------------------ updateUniformBuffer
    // Call every frame with the world-space position of every shadow-casting light, in the
    // same order as the lights in your GUBO (light i here = shadow layers 6i..6i+5).
    void updateUniformBuffer(int currentImage, const std::vector<glm::vec4> &lightPos) {
        if (lightPos.size() != prm.lightCount)
            throw std::runtime_error("ShadowMapper: updateUniformBuffer needs one position per light");

        ShadowUBO u{};
        u.params = glm::vec4(prm.nearPlane, prm.farPlane, (float)prm.lightCount,
                             1.0f - 2.0f * GuardTexels / (float)prm.resolution); // w = guard band scale
        for (uint32_t i = 0; i < prm.lightCount; i++)
            u.lightPos[i] = lightPos[i];

        // current world matrix of every caster (Instance::Wm)
        for (auto &c : casters) {
            u.model = c.inst->Wm;
            c.ds.map(currentImage, &u, 0);
        }
    }

    // ------------------------------------------------ command buffer
    // Record the shadow pass. Call BEFORE RP.begin(...) of your main pass,
    // inside the same command buffer (or in a command buffer with a lower order).
    void populateCommandBuffer(VkCommandBuffer cb, int currentImage) {
        for (uint32_t layer = 0; layer < layerCount; layer++) {
            renderPass.begin(cb, layer); // framebuffer #layer renders into shadow layer #layer

            pipeline.bind(cb);
            vkCmdPushConstants(cb, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &layer);
            for (auto &c : casters) {
                c.ds.bind(cb, pipeline, 0, currentImage);
                c.model->bind(cb);
                vkCmdDrawIndexed(cb, static_cast<uint32_t>(c.model->indices.size()), 1, 0, 0, 0);
            }

            renderPass.end(cb);
        }
    }

    // Use as the external (fromInstance = false) texture of your TechniqueRef:
    // it is the `sampler2DArrayShadow shadowMaps` binding of the scene DescriptorSet.
    VkDescriptorImageInfo getViewAndSampler() const {
        return {sampler, shadowView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
    }

    uint32_t getLightCount() const { return prm.lightCount; }

    // ------------------------------------- pipelinesAndDescriptorSetsCleanup
    void pipelinesAndDescriptorSetsCleanup() {
        pipeline.cleanup();
        renderPass.cleanup(); // framebuffers, VkRenderPass
        for (auto &c : casters)
            c.ds.cleanup();
    }

    // ------------------------------------------------------- localCleanup
    void localCleanup() {
        renderPass.destroy();
        pipeline.destroy();
        DSLshadow.cleanup();

        vkDestroySampler(device, sampler, nullptr);

        for (auto v : layerViews)
            vkDestroyImageView(device, v, nullptr);
        vkDestroyImageView(device, shadowView, nullptr);
        vkDestroyImage(device, shadowImage, nullptr);
        vkFreeMemory(device, shadowMem, nullptr);
    }

  private:

    struct Caster {
        Instance *inst = nullptr;
        Model *model = nullptr;
        DescriptorSet ds; // model matrix + light data, one UBO per swap-chain image
    };
    Scene *scene = nullptr;
    std::vector<Caster> casters;

    BaseProject *BP = nullptr;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VertexDescriptor *VD = nullptr;
    ShadowMapperParams prm;
    uint32_t layerCount = 0; // lightCount * 6

    VkImage shadowImage = VK_NULL_HANDLE; // depth, 2D array, layerCount layers
    VkDeviceMemory shadowMem = VK_NULL_HANDLE;
    VkImageView shadowView = VK_NULL_HANDLE; // 2D_ARRAY view, sampled by the scene shader
    std::vector<VkImageView> layerViews;     // one 2D view per layer, framebuffer targets
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    bool linearCompare = true; // depth format supports linear filtering (hardware PCF)

    VkSampler sampler = VK_NULL_HANDLE;

    DescriptorSetLayout DSLshadow;
    Pipeline pipeline;
    RenderPass renderPass{};

    // ---- Vulkan helpers (BaseProject's are protected) -----------------------
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) {
        VkPhysicalDeviceMemoryProperties mp;
        vkGetPhysicalDeviceMemoryProperties(physDevice, &mp);
        for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
            if ((typeFilter & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props)
                return i;
        throw std::runtime_error("ShadowMapper: no suitable memory type!");
    }

    void createImage(uint32_t layers, VkFormat fmt, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImage &img,
                     VkDeviceMemory &mem) {
        VkImageCreateInfo ii{};
        ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ii.flags = flags;
        ii.imageType = VK_IMAGE_TYPE_2D;
        ii.format = fmt;
        ii.extent = {prm.resolution, prm.resolution, 1};
        ii.mipLevels = 1;
        ii.arrayLayers = layers;
        ii.samples = VK_SAMPLE_COUNT_1_BIT;
        ii.tiling = VK_IMAGE_TILING_OPTIMAL;
        ii.usage = usage;
        ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkResult r = vkCreateImage(device, &ii, nullptr, &img);
        if (r != VK_SUCCESS) {
            PrintVkError(r);
            throw std::runtime_error("ShadowMapper: failed to create image!");
        }

        VkMemoryRequirements mr;
        vkGetImageMemoryRequirements(device, img, &mr);
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = mr.size;
        ai.memoryTypeIndex = findMemoryType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(device, &ai, nullptr, &mem) != VK_SUCCESS)
            throw std::runtime_error("ShadowMapper: failed to allocate image memory!");
        vkBindImageMemory(device, img, mem, 0);
    }

    VkImageView createView(VkImage img, VkFormat fmt, VkImageAspectFlags aspect, VkImageViewType type,
                           uint32_t baseLayer, uint32_t layers) {
        VkImageViewCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = img;
        vi.viewType = type;
        vi.format = fmt;
        vi.subresourceRange = {aspect, 0, 1, baseLayer, layers};
        VkImageView v;
        VkResult r = vkCreateImageView(device, &vi, nullptr, &v);
        if (r != VK_SUCCESS) {
            PrintVkError(r);
            throw std::runtime_error("ShadowMapper: failed to create image view!");
        }
        return v;
    }

    // depth format that can be both rendered to and sampled
    VkFormat pickDepthFormat() {
        const VkFormatFeatureFlags need =
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        for (VkFormat f : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
            VkFormatProperties fp;
            vkGetPhysicalDeviceFormatProperties(physDevice, f, &fp);
            if ((fp.optimalTilingFeatures & need) == need) {
                linearCompare = (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
                return f;
            }
        }
        throw std::runtime_error("ShadowMapper: no sampleable depth format available!");
    }

    void createShadowImages() {
        // 6 depth layers per light, rendered + sampled
        createImage(layerCount, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    0, shadowImage, shadowMem);
        shadowView =
            createView(shadowImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, layerCount);
        layerViews.resize(layerCount);
        for (uint32_t i = 0; i < layerCount; i++)
            layerViews[i] = createView(shadowImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, i, 1);
    }

    // Describes the pass to Starter's RenderPass: a depth-only pass (no colour attachment).
    // The VkRenderPass is created later by renderPass.create().
    void createRenderPass() {
        std::vector<AttachmentProperties> att = {
            // 0: depth, stored and left ready to be sampled
            {DEPTH_AT, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
             VK_IMAGE_ASPECT_DEPTH_BIT, false, false, {}, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
             VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
        att[0].clearValue.depthStencil = {1.0f, 0};

        std::vector<VkSubpassDependency> dep(2);
        // previous frame's sampling / previous layer's writes must finish before we overwrite
        dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dep[0].dstSubpass = 0;
        dep[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dep[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dep[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dep[0].dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // shadow writes must be visible to the main pass fragment shader
        dep[1].srcSubpass = 0;
        dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dep[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dep[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dep[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dep[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // count = layerCount: one framebuffer per layer, selected with renderPass.begin(cb, layer)
        renderPass.init(BP, (int)prm.resolution, (int)prm.resolution, (int)layerCount, &att, &dep);
    }

    // RenderPass::create() builds its `count` framebuffers on its own 2D attachment, which
    // cannot target the layers of the shadow image. Replace them with one framebuffer per
    // layer and release the placeholder depth image RenderPass allocated.
    void bindLayerFramebuffers() {
        for (auto fb : renderPass.frameBuffers)
            vkDestroyFramebuffer(device, fb, nullptr);

        FrameBufferAttachment &placeholder = renderPass.attachments[0];
        vkDestroyImageView(device, placeholder.view, nullptr);
        vkDestroyImage(device, placeholder.image, nullptr);
        vkFreeMemory(device, placeholder.mem, nullptr);
        // RenderPass::cleanup() destroys these again: harmless on null handles
        placeholder.view = VK_NULL_HANDLE;
        placeholder.image = VK_NULL_HANDLE;
        placeholder.mem = VK_NULL_HANDLE;

        for (uint32_t f = 0; f < layerCount; f++) {
            VkFramebufferCreateInfo fi{};
            fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fi.renderPass = renderPass.renderPass;
            fi.attachmentCount = 1;
            fi.pAttachments = &layerViews[f];
            fi.width = fi.height = prm.resolution;
            fi.layers = 1;
            VkResult r = vkCreateFramebuffer(device, &fi, nullptr, &renderPass.frameBuffers[f]);
            if (r != VK_SUCCESS) {
                PrintVkError(r);
                throw std::runtime_error("ShadowMapper: failed to create framebuffer!");
            }
        }
    }

    void createSampler() {
        // hardware depth comparison: one tap returns the (bilinear) fraction of lit texels
        VkSamplerCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        si.magFilter = si.minFilter = linearCompare ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.anisotropyEnable = VK_FALSE;
        si.maxAnisotropy = 1.0f;
        si.compareEnable = VK_TRUE;
        si.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // lit when ref <= stored depth
        si.minLod = 0.0f;
        si.maxLod = 0.0f;
        si.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VkResult r = vkCreateSampler(device, &si, nullptr, &sampler);
        if (r != VK_SUCCESS) {
            PrintVkError(r);
            throw std::runtime_error("ShadowMapper: failed to create sampler!");
        }
    }
};
