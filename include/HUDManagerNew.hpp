#pragma once

#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>

class HUDManagerNew {
  private:
    RenderPass rp;
    Pipeline p;
    VertexDescriptor vd;
    DescriptorSet ds;
    DescriptorSetLayout dsl;

  public:
    HUDManagerNew(BaseProject *bp, int w, int h);

    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void populateCommandBuffer(VkCommandBuffer cmdBuf, int currentImage);
    void updateUniformBuffer(int currentImage, float deltaT);
    void setPrompt(std::string_view text);
    void setStatus(std::string_view text);

    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
