#pragma once

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

class SceneManager {
private:
    static constexpr int textureSize = 4;
    BaseProject *bp;
    int w, h;

    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLlocal, DSLglobal;

    // Vertex descriptors, Pipelines [Shader couples] and Render pass
    VertexDescriptor VD;
    RenderPass RP;
    Pipeline P;

    Scene scene;
    std::vector<VertexDescriptorRef> vdrs;
    std::vector<TechniqueRef> prs;

    static std::vector<PipelineAndTexturesDefs> generateTextureDef(Pipeline *P, int numTextures);
public:
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);

    SceneManager(BaseProject *bp, std::string_view scene_json, std::string_view vert_shader,
                 std::string_view frag_shader, int numTextures, int w, int h);
    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);
    void updateUniformBuffer(uint32_t currentImage, glm::vec4 debugView, glm::mat4 viewPrjMat, glm::vec3 cameraPos, float deltaT);

    // Returns the collider of the instance with the given id (nullptr if there is none)
    Collider *getCollider(const std::string &instanceId);
    // Moves the instance (and its collider) out of the map
    void hideInstance(const std::string &instanceId);

    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
