#include "HUDManagerNew.hpp"
#include "modules/Starter.hpp"
#include <vulkan/vulkan_core.h>

HUDManagerNew::HUDManagerNew(BaseProject *bp, int w, int h) {
    vd.init(bp, {}, {});
    p.init(bp, &vd, "shaders/hud.vert.spv", "shaders/hud.frag.spv", {});
    p.setCullMode(VK_CULL_MODE_NONE);
    p.setTransparency(true); // alpha blending, useful for HUD elements
    p.setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    // Stock config = [swapchain color (1 sample), depth]; keep only the color one
    AttachmentProperties color = (*RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, bp))[0];
    color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;             // keep the scene
    color.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // as left by the scene pass
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    std::vector<AttachmentProperties> props = {color};

    // Wait for the scene pass's color writes before loading/blending on top of them
    std::vector<VkSubpassDependency> deps = {
        {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0}};

    rp.init(bp, -1, -1, -1, &props, &deps);
}

void HUDManagerNew::onResize(int w, int h) {
    rp.width = w;
    rp.height = h;
}

void HUDManagerNew::pipelinesAndDescriptorSetsInit() {
    rp.create();
    p.create(&rp);
}

void HUDManagerNew::populateCommandBuffer(VkCommandBuffer cmdBuf, int currentImage) {
    rp.begin(cmdBuf, currentImage);
    p.bind(cmdBuf);
    vkCmdDraw(cmdBuf, 4, 1, 0, 0);
    // vkCmdDraw(cmdBuf, 3, 1, 3, 0);
    rp.end(cmdBuf);
}

void HUDManagerNew::updateUniformBuffer(int currentImage, float deltaT) {}

void HUDManagerNew::setPrompt(std::string_view text) {}

void HUDManagerNew::setStatus(std::string_view text) {}

void HUDManagerNew::cleanUp() {
    p.destroy();
    rp.destroy();
}

void HUDManagerNew::pipelinesAndDescriptorSetsCleanup() {
    p.cleanup();
    rp.cleanup();
}
