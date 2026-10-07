#pragma once

#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>
#include <modules/Scene.hpp>

class SceneManager {
private:
    const int textureSize = 4;
    BaseProject *bp;
    Scene scene;
    int w, h;

    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    std::unique_ptr<DescriptorSetLayout> DSLlocal, DSLglobal;

    // Vertex descriptors, Pipelines [Shader couples] and Render pass
    std::unique_ptr<VertexDescriptor> VD;
    std::unique_ptr<RenderPass> RP;
    std::unique_ptr<Pipeline> P;

    // Models, textures and Descriptors (values assigned to the uniforms)
    std::unique_ptr<Model> MSphere, Mplane, MCube, MSoftbal, MStatue;
    std::unique_ptr<Texture[]> Talbedo, TNorm, Tmetal, Troughness, Tao;
    std::unique_ptr<DescriptorSet> DSglobal;
    std::unique_ptr<DescriptorSet> DSlocalSphere, DSlocalCube, DSlocalSoftbal;
    std::unique_ptr<DescriptorSet> DSlocalStatue, DSlocalPlane;

    // to provide textual feedback
    std::unique_ptr<TextMaker> txt;
public:
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);

    SceneManager(BaseProject *bp, std::string_view scene_json, int w, int h);
    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);
    void updateUniformBuffer(uint32_t currentImage, glm::vec4 debugView, glm::mat4 viewPrjMat, glm::vec3 cameraPos, float deltaT);
    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
