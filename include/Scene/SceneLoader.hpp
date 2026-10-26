#pragma once

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include "structs.hpp"
#include "CollisionDetector.hpp"

class SceneLoader {
  private:
    BaseProject *bp;
    int w, h;

    std::map<std::string, DescriptorSetLayout> dslMap;
    std::map<std::string, VertexDescriptor> vdMap;
    std::map<std::string, std::vector<Pipeline>> pMap;

    Scene scene;

    std::vector<VertexDescriptorRef> vdrs;
    std::vector<TechniqueRef> prs;
    std::vector<Instance *> instances;

    std::unique_ptr<CollisionDetector> collisionDetector;

    static std::vector<PipelineAndTexturesDefs> generateTextureDef(std::vector<Pipeline> &pipelines, int numTextures);

    void generateVertexDescriptors(nlohmann::json &jsonFile);
    void generateDescriptorSetLayout(nlohmann::json &technique, std::size_t guboSize, std::size_t uboSize,
                                     int numTextures);
    void generatePipelines(nlohmann::json &jsonFile, std::size_t guboSize, std::size_t uboSize);

  public:
    SceneLoader(BaseProject *bp, std::string_view scene_json, int w, int h, std::size_t guboSize, std::size_t uboSize);

    void onResize(int newW, int newH);

    void pipelinesAndDescriptorSetsInit(RenderPass &rp);

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat);

    const std::vector<Instance *> &getInstances();

    CollisionDetector *getCollisionDetector() const;

    void refreshColliders();

    void cleanUp();

    void pipelinesAndDescriptorSetsCleanup();
};
