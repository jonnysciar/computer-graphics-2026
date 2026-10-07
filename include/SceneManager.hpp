#pragma once

#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>
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

    // Models, textures and Descriptors (values assigned to the uniforms)
    DescriptorSet DSglobal;

    // to provide textual feedback
    TextMaker txt;
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
