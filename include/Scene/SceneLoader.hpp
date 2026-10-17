#pragma once

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include "structs.hpp"

class SceneLoader {
  private:
    BaseProject *bp;
    int w, h;

    std::map<std::string, DescriptorSetLayout> dslMap;
    std::map<std::string, VertexDescriptor> vdMap;
    std::map<std::string, std::vector<Pipeline>> pMap;

    std::vector<RenderPass> rps;
    Scene scene;

    std::vector<VertexDescriptorRef> vdrs;
    std::vector<TechniqueRef> prs;
    std::vector<Instance *> instances;

    static std::vector<PipelineAndTexturesDefs> generateTextureDef(std::vector<Pipeline> &pipelines, int numTextures);

    void generateVertexDescriptors(nlohmann::json &jsonFile);
    void generateDescriptorSetLayout(nlohmann::json &technique, int guboSize, int uboSize, int numTextures);
    void generatePipelines(nlohmann::json &jsonFile, int guboSize, int uboSize);

  public:
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);

    SceneLoader(BaseProject *bp, std::string_view scene_json, int w, int h, int guboSize, int uboSize);

    void onResize(int newW, int newH);

    void pipelinesAndDescriptorSetsInit();

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat);

    const std::vector<Instance *> &getInstances();

    void refreshColliders();

    void cleanUp();

    void pipelinesAndDescriptorSetsCleanup();
};
