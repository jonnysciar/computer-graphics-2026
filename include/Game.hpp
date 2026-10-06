#pragma once

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

// The uniform buffer object used in this example
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 ambientUpper; // xyz = sky / upper  color  (lU)
    alignas(16) glm::vec4 ambientLower; // xyz = ground / lower color (lD)
    alignas(16) glm::vec4 ambientDir; // xyz = "up" direction for blending (d)
    alignas(16) glm::vec4 debugView;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
    glm::vec4 tan;
};

class Game : public BaseProject {
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLlocal, DSLglobal;

    // Vertex descriptors, Pipelines [Shader couples] and Render pass
    VertexDescriptor VD;
    RenderPass RP;
    Pipeline P;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model MSphere, Mplane, MCube, MSoftbal, MStatue;
    Texture Talbedo[4], TNorm[4], Tmetal[4], Troughness[4], Tao[4];
    DescriptorSet DSglobal;
    DescriptorSet DSlocalSphere, DSlocalCube, DSlocalSoftbal;
    DescriptorSet DSlocalStatue, DSlocalPlane;

    // to provide textual feedback
    TextMaker txt;

    // Other application parameters
    float Ar; // Aspect ratio

    glm::mat4 ViewPrj;
    glm::vec3 cameraPos;

    glm::vec4 debugView = glm::vec4(0.0);

    // What to do when the window changes size
    void onWindowResize(int w, int h) override;

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() override;

    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() override;

    void updateUniformBuffer(uint32_t currentImage) override;

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    static void populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params);

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

    void localCleanup() override;

    // Here you destroy your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsCleanup() override;

public:
    // Here you set the main application parameters
    void setWindowParameters() override;
};
