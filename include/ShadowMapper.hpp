#pragma once

#include "CameraController.hpp"
#include "uniform_buffers.hpp"

#include <functional>

struct ShadowMapperParams {
    uint32_t resolution = 512; // size of each cube face
    float nearPlane = CameraController::nearPlane;
    float farPlane = CameraController::farPlane; // must cover everything the light reaches
    std::string vertShader = "shaders/shadow.vert.spv";
    std::string fragShader = "shaders/shadow.frag.spv";
};

class ShadowMapper {
  public:
    // Return false to exclude an instance from casting shadows
    // (e.g. the lamp mesh that surrounds the light).
    using CasterFilter = std::function<bool(const Instance &)>;

    ShadowMapper() = default;

    void init(BaseProject *bp, VertexDescriptor *vd, VkDevice dev, VkPhysicalDevice physicalDev,
              ShadowMapperParams p = {}) {
        BP = bp;
        device = dev;
        physDevice = physicalDev;
        VD = vd;
        prm = std::move(p);

        createShadowImages();
        createRenderPass();
        createFramebuffers();
        createSampler();

        DSLshadow.init(
            BP, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                  static_cast<int>(sizeof(ShadowUBO)), 1}});

        VkPushConstantRange pk{};
        pk.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pk.offset = 0;
        pk.size = sizeof(uint32_t); // cube face index

        P.init(BP, VD, prm.vertShader, prm.fragShader, {&DSLshadow}, {pk});
        P.setCullMode(VK_CULL_MODE_NONE); // robust against the cube-map winding flip

        // Minimal description of our render pass for Pipeline::create()
        // (Starter's RenderPass object is NOT used to create anything).
        rpInfo.BP = BP;
        rpInfo.width = rpInfo.height = (int)prm.resolution;
        rpInfo.count = 6;
        rpInfo.renderPass = vkRP;
        rpInfo.colorAttchementsCount = 1;
        rpInfo.firstColorAttIdx = -1; // -> 1 sample
        rpInfo.depthAttIdx = 1;
        rpInfo.resolveAttIdx = -1;
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
        BP->DPSZs.uniformBlocksInPool += 1 + (int)casters.size();
        BP->DPSZs.setsInPool += 1 + (int)casters.size();
        std::cout << "ShadowMapper: " << casters.size() << " shadow casters\n";
    }

    // ------------------------------------- pipelinesAndDescriptorSetsInit
    void pipelinesAndDescriptorSetsInit() {
        P.create(&rpInfo);
        DSshadow.init(BP, &DSLshadow, {});
        for (auto &c : casters)
            c.ds.init(BP, &DSLshadow, {});
    }

    // ------------------------------------------------ updateUniformBuffer
    // Call every frame with the light position (world space): the same value
    // you put in gubo.lightPos.
    void updateUniformBuffer(int currentImage, const glm::vec3 &lightPos) {
        ShadowUBO u{};
        for (int f = 0; f < 6; f++)
            u.faceVP[f] = faceViewProj(f, lightPos, prm.nearPlane, prm.farPlane);
        u.lightPosFar = glm::vec4(lightPos, prm.farPlane);
        DSshadow.map(currentImage, &u, 0);

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
        VkClearValue clear[2]{};
        clear[0].color = {{1.0e30f, 0.0f, 0.0f, 0.0f}}; // "nothing hit" = very far
        clear[1].depthStencil = {1.0f, 0};

        for (uint32_t face = 0; face < 6; face++) {
            VkRenderPassBeginInfo bi{};
            bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            bi.renderPass = vkRP;
            bi.framebuffer = framebuffers[face];
            bi.renderArea.offset = {0, 0};
            bi.renderArea.extent = {prm.resolution, prm.resolution};
            bi.clearValueCount = 2;
            bi.pClearValues = clear;
            vkCmdBeginRenderPass(cb, &bi, VK_SUBPASS_CONTENTS_INLINE);

            P.bind(cb);
            DSshadow.bind(cb, P, 0, currentImage);
            vkCmdPushConstants(cb, P.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &face);
            for (auto &c : casters) {
                c.ds.bind(cb, P, 0, currentImage);
                c.model->bind(cb);
                vkCmdDrawIndexed(cb, static_cast<uint32_t>(c.model->indices.size()), 1, 0, 0, 0);
            }

            vkCmdEndRenderPass(cb);
        }
    }

    // Use as the external (fromInstance = false) texture of your TechniqueRef:
    // it is the samplerCube binding of the scene DescriptorSet.
    VkDescriptorImageInfo getViewAndSampler() const {
        return {sampler, cubeView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    // ------------------------------------- pipelinesAndDescriptorSetsCleanup
    void pipelinesAndDescriptorSetsCleanup() {
        P.cleanup();
        DSshadow.cleanup();
        for (auto &c : casters)
            c.ds.cleanup();
    }

    // ------------------------------------------------------- localCleanup
    void localCleanup() {
        P.destroy();
        DSLshadow.cleanup();

        vkDestroySampler(device, sampler, nullptr);
        for (auto fb : framebuffers)
            vkDestroyFramebuffer(device, fb, nullptr);
        vkDestroyRenderPass(device, vkRP, nullptr);

        for (auto v : faceViews)
            vkDestroyImageView(device, v, nullptr);
        vkDestroyImageView(device, cubeView, nullptr);
        vkDestroyImage(device, cubeImage, nullptr);
        vkFreeMemory(device, cubeMem, nullptr);

        vkDestroyImageView(device, depthView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthMem, nullptr);
    }

  private:
    struct Caster {
        Instance *inst = nullptr;
        Model *model = nullptr;
        DescriptorSet ds; // set 1: model matrix, one UBO per swap-chain image
    };
    Scene *scene = nullptr;
    std::vector<Caster> casters;

    BaseProject *BP = nullptr;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VertexDescriptor *VD = nullptr;
    ShadowMapperParams prm;

    VkImage cubeImage = VK_NULL_HANDLE;
    VkDeviceMemory cubeMem = VK_NULL_HANDLE;
    VkImageView cubeView = VK_NULL_HANDLE;
    VkImageView faceViews[6] = {};
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthMem = VK_NULL_HANDLE;
    VkImageView depthView = VK_NULL_HANDLE;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    static constexpr VkFormat distFormat = VK_FORMAT_R32_SFLOAT;

    VkRenderPass vkRP = VK_NULL_HANDLE;
    VkFramebuffer framebuffers[6] = {};
    VkSampler sampler = VK_NULL_HANDLE;

    DescriptorSetLayout DSLshadow;
    DescriptorSet DSshadow;
    Pipeline P;
    RenderPass rpInfo{};

    // ---- cube face matrices ------------------------------------------------
    // Built directly from the Vulkan cube-map face selection table, so that
    // texture(samplerCube, dir) in the scene shader hits the right texel.
    // Clip space is Vulkan's (y down, depth 0..1): no extra projection flip.
    static glm::mat4 faceViewProj(int f, const glm::vec3 &L, float n, float fa) {
        static const glm::vec3 R[6] = {{0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {-1, 0, 0}};    // sc
        static const glm::vec3 U[6] = {{0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}}; // tc
        static const glm::vec3 F[6] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                       {0, -1, 0}, {0, 0, 1},  {0, 0, -1}}; // major axis

        glm::mat4 V(1.0f);
        V[0][0] = R[f].x;
        V[1][0] = R[f].y;
        V[2][0] = R[f].z;
        V[3][0] = -glm::dot(R[f], L);
        V[0][1] = U[f].x;
        V[1][1] = U[f].y;
        V[2][1] = U[f].z;
        V[3][1] = -glm::dot(U[f], L);
        V[0][2] = F[f].x;
        V[1][2] = F[f].y;
        V[2][2] = F[f].z;
        V[3][2] = -glm::dot(F[f], L);

        glm::mat4 Pm(0.0f); // 90 deg fov, aspect 1, depth 0..1
        Pm[0][0] = 1.0f;
        Pm[1][1] = 1.0f;
        Pm[2][2] = fa / (fa - n);
        Pm[3][2] = -fa * n / (fa - n);
        Pm[2][3] = 1.0f; // w = forward distance
        return Pm * V;
    }

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

    VkFormat pickDepthFormat() {
        for (VkFormat f : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
            VkFormatProperties fp;
            vkGetPhysicalDeviceFormatProperties(physDevice, f, &fp);
            if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return f;
        }
        throw std::runtime_error("ShadowMapper: no depth format available!");
    }

    void createShadowImages() {
        // cube of distances: 6 layers, cube-compatible, rendered + sampled
        createImage(6, distFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, cubeImage, cubeMem);
        cubeView = createView(cubeImage, distFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 0, 6);
        for (uint32_t i = 0; i < 6; i++)
            faceViews[i] = createView(cubeImage, distFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, i, 1);

        // one depth buffer shared by the 6 sequential face passes
        depthFormat = pickDepthFormat();
        createImage(1, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, depthImage, depthMem);
        depthView = createView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1);
    }

    void createRenderPass() {
        VkAttachmentDescription att[2]{};
        att[0].format = distFormat;
        att[0].samples = VK_SAMPLE_COUNT_1_BIT;
        att[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // ready to be sampled

        att[1].format = depthFormat;
        att[1].samples = VK_SAMPLE_COUNT_1_BIT;
        att[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;
        sub.pDepthStencilAttachment = &depthRef;

        VkSubpassDependency dep[2]{};
        // previous frame's sampling / previous face's writes must finish before we overwrite
        dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dep[0].dstSubpass = 0;
        dep[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // shadow writes must be visible to the main pass fragment shader
        dep[1].srcSubpass = 0;
        dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dep[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dep[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        VkRenderPassCreateInfo ri{};
        ri.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ri.attachmentCount = 2;
        ri.pAttachments = att;
        ri.subpassCount = 1;
        ri.pSubpasses = &sub;
        ri.dependencyCount = 2;
        ri.pDependencies = dep;
        VkResult r = vkCreateRenderPass(device, &ri, nullptr, &vkRP);
        if (r != VK_SUCCESS) {
            PrintVkError(r);
            throw std::runtime_error("ShadowMapper: failed to create render pass!");
        }
    }

    void createFramebuffers() {
        for (int f = 0; f < 6; f++) {
            VkImageView views[2] = {faceViews[f], depthView};
            VkFramebufferCreateInfo fi{};
            fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fi.renderPass = vkRP;
            fi.attachmentCount = 2;
            fi.pAttachments = views;
            fi.width = fi.height = prm.resolution;
            fi.layers = 1;
            VkResult r = vkCreateFramebuffer(device, &fi, nullptr, &framebuffers[f]);
            if (r != VK_SUCCESS) {
                PrintVkError(r);
                throw std::runtime_error("ShadowMapper: failed to create framebuffer!");
            }
        }
    }

    void createSampler() {
        // R32_SFLOAT is not guaranteed to support linear filtering -> NEAREST;
        // smoothing is done by PCF in the scene fragment shader.
        VkSamplerCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        si.magFilter = si.minFilter = VK_FILTER_NEAREST;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.anisotropyEnable = VK_FALSE;
        si.maxAnisotropy = 1.0f;
        si.compareEnable = VK_FALSE;
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
