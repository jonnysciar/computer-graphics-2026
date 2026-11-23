#pragma once

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include "CameraController.hpp"
#include "GameStatus.hpp"
#include "CollisionDetector.hpp"
#include "ShadowMapper.hpp"

class SceneLoader {
  private:
    BaseProject *bp;
    int w, h;

    RenderPass rp{};

    std::map<std::string, DescriptorSetLayout> dslMap;
    std::map<std::string, VertexDescriptor> vdMap;
    std::map<std::string, std::vector<Pipeline>> pMap;
    std::map<std::string, ShadowMapper> smMap;

    Scene scene;

    std::vector<VertexDescriptorRef> vdrs;
    std::vector<TechniqueRef> prs;
    std::vector<Instance *> instances;
    std::map<std::string, std::vector<Instance *>> techInstMap;

    std::unique_ptr<CollisionDetector> collisionDetector;

    // footprint of each room, from the colliders of its walls and roof
    std::map<int, AABBextents> roomBounds;
    static std::vector<PipelineAndTexturesDefs> generateTextureDef(std::string_view tech_name,
                                                                   std::vector<Pipeline> &pipelines, int numTextures);

    void generateVertexDescriptors(nlohmann::json &jsonFile);
    void generateDescriptorSetLayout(nlohmann::json &technique, int numTextures);
    void generatePipelines(nlohmann::json &jsonFile, VkDevice dev, VkPhysicalDevice physicalDev);

    void computeRoomBounds();
    void updatePlayerRoom(GameStatus &gameStatus) const;
    void generalUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController);
    void emissiveUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController);
    void armsUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController);

  public:
    SceneLoader(BaseProject *bp, std::string_view scene_json, VkDevice dev, VkPhysicalDevice physicalDev, int w, int h);

    void onResize(int newW, int newH);

    void pipelinesAndDescriptorSetsInit();

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void populateShadowCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void updateUniformBuffer(int currentImage, GameStatus &gameStatus, CameraController &cameraController);

    // Chooses the shadow casters of each torch and writes the uniforms of the shadow pass
    void prepareShadowPass(int currentImage, GameStatus &gameStatus);

    const std::vector<Instance *> &getInstances();

    CollisionDetector *getCollisionDetector() const;

    void refreshColliders();

    void cleanUp();

    void pipelinesAndDescriptorSetsCleanup();
};
