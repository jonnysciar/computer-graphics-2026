#pragma once

#include <modules/Starter.hpp>
#include <modules/Scene.hpp>

#include  "structs.hpp"

class SceneLoader {
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

    SceneLoader(BaseProject *bp, std::string_view scene_json, std::string_view vert_shader,
                std::string_view frag_shader, int numTextures, int w, int h, int guboSize, int uboSize);

    void onResize(int newW, int newH);

    void pipelinesAndDescriptorSetsInit();

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat);

    std::vector<Instance *> getInstances() const;

    void refreshColliders();

    void cleanUp();

    void pipelinesAndDescriptorSetsCleanup();
};
