#include "SceneManager.hpp"

#include "structs.hpp"

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"

void SceneManager::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
    auto *SM = static_cast<SceneManager *>(Params);
    SM->populateCommandBuffer(commandBuffer, currentImage);
}

SceneManager::SceneManager(BaseProject *bp, std::string_view scene_json, int w, int h) : bp(bp), w(w), h(h) {
    // Descriptor Layouts [what will be passed to the shaders]
    DSLlocal.init(this->bp, {
                      {
                          0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,
                          sizeof(UniformBufferObject), 1
                      },
                      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4},
                      {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 4},
                      {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8, 4},
                      {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 12, 4},
                      {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 16, 4}
                  });
    DSLglobal.init(this->bp, {
                       {
                           0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS,
                           sizeof(GlobalUniformBufferObject), 1
                       }
                   });
    VD.init(this->bp, {
                {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
                {
                    0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                    sizeof(glm::vec3), POSITION
                },
                {
                    0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                    sizeof(glm::vec3), NORMAL
                },
                {
                    0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                    sizeof(glm::vec2), UV
                },
                {
                    0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tan),
                    sizeof(glm::vec4), TANGENT
                }
            });
    // initializes the render pass
    RP.init(this->bp);
    RP.properties[0].clearValue = {0.08f, 0.10f, 0.16f, 1.0f};

    // Pipelines [Shader couples]
    P.init(this->bp, &VD, "shaders/MeshTBN.vert.spv", "shaders/AdvancedTexturingExercise.frag.spv",
           {&DSLglobal, &DSLlocal});

    // Models, textures and Descriptors (values assigned to the uniforms)
    MSphere.init(this->bp, &VD, "assets/models/Sphere.gltf", GLTF);
    MCube.init(this->bp, &VD, "assets/models/Cube.gltf", GLTF);
    MSoftbal.init(this->bp, &VD, "assets/models/Softball.gltf", GLTF);
    MStatue.init(this->bp, &VD, "assets/models/Statue.gltf", GLTF);
    Mplane.init(this->bp, &VD, "assets/models/Map.gltf", GLTF);

    Talbedo[0].init(this->bp, "assets/textures/ice-field/ice_field_albedo.png");
    TNorm[0].init(this->bp, "assets/textures/ice-field/ice_field_normal-ogl.png", VK_FORMAT_R8G8B8A8_UNORM);
    Tmetal[0].init(this->bp, "assets/textures/ice-field/ice_field_metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
    Troughness[0].init(this->bp, "assets/textures/ice-field/ice_field_roughness.png", VK_FORMAT_R8G8B8A8_UNORM);
    Tao[0].init(this->bp, "assets/textures/ice-field/ice_field_ao.png", VK_FORMAT_R8G8B8A8_UNORM);
    Talbedo[1].init(this->bp, "assets/textures/rock-wall-mortar/rock-wall-mortar_albedo.png");
    TNorm[1].init(this->bp, "assets/textures/rock-wall-mortar/rock-wall-mortar_normal-ogl.png",
                  VK_FORMAT_R8G8B8A8_UNORM);
    Tmetal[1].init(this->bp, "assets/textures/rock-wall-mortar/rock-wall-mortar_metallic.png",
                   VK_FORMAT_R8G8B8A8_UNORM);
    Troughness[1].init(this->bp, "assets/textures/rock-wall-mortar/rock-wall-mortar_roughness.png",
                       VK_FORMAT_R8G8B8A8_UNORM);
    Tao[1].init(this->bp, "assets/textures/rock-wall-mortar/rock-wall-mortar_ao.png", VK_FORMAT_R8G8B8A8_UNORM);
    Talbedo[2].init(this->bp, "assets/textures/granite-tile/granite-tile_albedo.png");
    TNorm[2].init(this->bp, "assets/textures/granite-tile/granite-tile_normal-ogl.png", VK_FORMAT_R8G8B8A8_UNORM);
    Tmetal[2].init(this->bp, "assets/textures/granite-tile/granite-tile_metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
    Troughness[2].init(this->bp, "assets/textures/granite-tile/granite-tile_roughness.png", VK_FORMAT_R8G8B8A8_UNORM);
    Tao[2].init(this->bp, "assets/textures/granite-tile/granite-tile_ao.png", VK_FORMAT_R8G8B8A8_UNORM);
    Talbedo[3].init(this->bp, "assets/textures/clay-shingles1/clay-shingles1_albedo.png");
    TNorm[3].init(this->bp, "assets/textures/clay-shingles1/clay-shingles1_normal-ogl.png", VK_FORMAT_R8G8B8A8_UNORM);
    Tmetal[3].init(this->bp, "assets/textures/clay-shingles1/clay-shingles1_metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
    Troughness[3].init(this->bp, "assets/textures/clay-shingles1/clay-shingles1_roughness.png",
                       VK_FORMAT_R8G8B8A8_UNORM);
    Tao[3].init(this->bp, "assets/textures/clay-shingles1/clay-shingles1_ao.png", VK_FORMAT_R8G8B8A8_UNORM);

    // sets the size of the Descriptor Set Pool
    this->bp->DPSZs.uniformBlocksInPool = 20;
    this->bp->DPSZs.texturesInPool = 120;
    this->bp->DPSZs.setsInPool = 20;

    // initializes the textual output
    txt.init(this->bp, w, h);

    // submits the main command buffer
    bp->submitCommandBuffer("main", 0, populateCommandBufferAccess, this);

    txt.print(1.0f, 1.0f, "FPS:", 1, "CO", false, false, true, TAL_RIGHT, TRH_RIGHT, TRV_BOTTOM,
              {1.0f, 0.0f, 0.0f, 1.0f}, {0.8f, 0.8f, 0.0f, 1.0f});
    txt.print(-1.0f, -1.0f, "3 - Change texture", 3);
}

void SceneManager::onResize(int w, int h) {
    RP.width = w;
    RP.height = h;
    txt.resizeScreen(w, h);
}

void SceneManager::pipelinesAndDescriptorSetsInit() {
    RP.create();

    P.create(&RP);

    DSglobal.init(this->bp, &DSLglobal, {});

    DSlocalSphere.init(this->bp, &DSLlocal, {
                           Talbedo[0].getViewAndSampler(), Talbedo[1].getViewAndSampler(),
                           Talbedo[2].getViewAndSampler(), Talbedo[3].getViewAndSampler(),
                           TNorm[0].getViewAndSampler(), TNorm[1].getViewAndSampler(), TNorm[2].getViewAndSampler(),
                           TNorm[3].getViewAndSampler(),
                           Tmetal[0].getViewAndSampler(), Tmetal[1].getViewAndSampler(),
                           Tmetal[2].getViewAndSampler(), Tmetal[3].getViewAndSampler(),
                           Troughness[0].getViewAndSampler(), Troughness[1].getViewAndSampler(),
                           Troughness[2].getViewAndSampler(), Troughness[3].getViewAndSampler(),
                           Tao[0].getViewAndSampler(), Tao[1].getViewAndSampler(), Tao[2].getViewAndSampler(),
                           Tao[3].getViewAndSampler()
                       });
    DSlocalCube.init(this->bp, &DSLlocal, {
                         Talbedo[0].getViewAndSampler(), Talbedo[1].getViewAndSampler(),
                         Talbedo[2].getViewAndSampler(), Talbedo[3].getViewAndSampler(),
                         TNorm[0].getViewAndSampler(), TNorm[1].getViewAndSampler(), TNorm[2].getViewAndSampler(),
                         TNorm[3].getViewAndSampler(),
                         Tmetal[0].getViewAndSampler(), Tmetal[1].getViewAndSampler(),
                         Tmetal[2].getViewAndSampler(), Tmetal[3].getViewAndSampler(),
                         Troughness[0].getViewAndSampler(), Troughness[1].getViewAndSampler(),
                         Troughness[2].getViewAndSampler(), Troughness[3].getViewAndSampler(),
                         Tao[0].getViewAndSampler(), Tao[1].getViewAndSampler(), Tao[2].getViewAndSampler(),
                         Tao[3].getViewAndSampler()
                     });
    DSlocalSoftbal.init(this->bp, &DSLlocal, {
                            Talbedo[0].getViewAndSampler(), Talbedo[1].getViewAndSampler(),
                            Talbedo[2].getViewAndSampler(), Talbedo[3].getViewAndSampler(),
                            TNorm[0].getViewAndSampler(), TNorm[1].getViewAndSampler(),
                            TNorm[2].getViewAndSampler(), TNorm[3].getViewAndSampler(),
                            Tmetal[0].getViewAndSampler(), Tmetal[1].getViewAndSampler(),
                            Tmetal[2].getViewAndSampler(), Tmetal[3].getViewAndSampler(),
                            Troughness[0].getViewAndSampler(), Troughness[1].getViewAndSampler(),
                            Troughness[2].getViewAndSampler(), Troughness[3].getViewAndSampler(),
                            Tao[0].getViewAndSampler(), Tao[1].getViewAndSampler(), Tao[2].getViewAndSampler(),
                            Tao[3].getViewAndSampler()
                        });
    DSlocalStatue.init(this->bp, &DSLlocal, {
                           Talbedo[0].getViewAndSampler(), Talbedo[1].getViewAndSampler(),
                           Talbedo[2].getViewAndSampler(), Talbedo[3].getViewAndSampler(),
                           TNorm[0].getViewAndSampler(), TNorm[1].getViewAndSampler(), TNorm[2].getViewAndSampler(),
                           TNorm[3].getViewAndSampler(),
                           Tmetal[0].getViewAndSampler(), Tmetal[1].getViewAndSampler(),
                           Tmetal[2].getViewAndSampler(), Tmetal[3].getViewAndSampler(),
                           Troughness[0].getViewAndSampler(), Troughness[1].getViewAndSampler(),
                           Troughness[2].getViewAndSampler(), Troughness[3].getViewAndSampler(),
                           Tao[0].getViewAndSampler(), Tao[1].getViewAndSampler(), Tao[2].getViewAndSampler(),
                           Tao[3].getViewAndSampler()
                       });
    DSlocalPlane.init(this->bp, &DSLlocal, {
                          Talbedo[0].getViewAndSampler(), Talbedo[1].getViewAndSampler(),
                          Talbedo[2].getViewAndSampler(), Talbedo[3].getViewAndSampler(),
                          TNorm[0].getViewAndSampler(), TNorm[1].getViewAndSampler(), TNorm[2].getViewAndSampler(),
                          TNorm[3].getViewAndSampler(),
                          Tmetal[0].getViewAndSampler(), Tmetal[1].getViewAndSampler(),
                          Tmetal[2].getViewAndSampler(), Tmetal[3].getViewAndSampler(),
                          Troughness[0].getViewAndSampler(), Troughness[1].getViewAndSampler(),
                          Troughness[2].getViewAndSampler(), Troughness[3].getViewAndSampler(),
                          Tao[0].getViewAndSampler(), Tao[1].getViewAndSampler(), Tao[2].getViewAndSampler(),
                          Tao[3].getViewAndSampler()
                      });
    txt.pipelinesAndDescriptorSetsInit();
}

void SceneManager::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    RP.begin(commandBuffer, currentImage);

    P.bind(commandBuffer);
    DSglobal.bind(commandBuffer, P, 0, currentImage);

    MSphere.bind(commandBuffer);
    DSlocalSphere.bind(commandBuffer, P, 1, currentImage);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSphere.indices.size()), 1, 0, 0, 0);

    MCube.bind(commandBuffer);
    DSlocalCube.bind(commandBuffer, P, 1, currentImage);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MCube.indices.size()), 1, 0, 0, 0);

    MSoftbal.bind(commandBuffer);
    DSlocalSoftbal.bind(commandBuffer, P, 1, currentImage);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSoftbal.indices.size()), 1, 0, 0, 0);

    MStatue.bind(commandBuffer);
    DSlocalStatue.bind(commandBuffer, P, 1, currentImage);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MStatue.indices.size()), 1, 0, 0, 0);

    Mplane.bind(commandBuffer);
    DSlocalPlane.bind(commandBuffer, P, 1, currentImage);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Mplane.indices.size()), 1, 0, 0, 0);

    RP.end(commandBuffer);
}

// TODO Maybe think a better way to pass dependencies
void SceneManager::updateUniformBuffer(uint32_t currentImage, glm::vec4 debugView, glm::mat4 viewPrjMat, glm::vec3 cameraPos, float deltaT) {
    // defines the global parameters for the uniform
    static float lightRotationAngle = 0.0f;
    //lightRotationAngle += 10.0f * deltaT;

    const glm::mat4 lightView = glm::rotate(glm::mat4(1), glm::radians(lightRotationAngle),
                                            glm::vec3(0.0f, 1.0f, 0.0f)) *
                                glm::rotate(glm::mat4(1), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::vec3 lightDir = glm::vec3(lightView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

    GlobalUniformBufferObject gubo{};
    gubo.lightDir = lightDir;
    gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 5.0f;
    gubo.eyePos = cameraPos;
    gubo.ambientUpper = glm::vec4(0.5f, 0.65f, 1.0f, 0.0f); // sky blue
    gubo.ambientLower = glm::vec4(0.2f, 0.15f, 0.1f, 0.0f); // warm ground
    gubo.ambientDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // world up
    gubo.debugView = debugView;
    DSglobal.map(currentImage, &gubo, 0);

    UniformBufferObject ubo{};

    ubo.mMat = glm::scale(glm::mat4(1), glm::vec3(16.0));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    DSlocalPlane.map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(-6, 1, 0));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    DSlocalSphere.map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(6, 1, 0));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    DSlocalCube.map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(-3, 1, -5));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    DSlocalSoftbal.map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(3, 0, -5)) * glm::scale(glm::mat4(1), glm::vec3(2.61));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    DSlocalStatue.map(currentImage, &ubo, 0);

    // updates the FPS counter
    static float elapsedT = 0.0f;
    static int countedFrames = 0;
    countedFrames++;
    elapsedT += deltaT;
    if (elapsedT > 1.0f) {
        float Fps = (float) countedFrames / elapsedT;
        std::ostringstream oss;
        oss << "FPS: " << Fps << "\n";
        txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true, TAL_RIGHT, TRH_RIGHT, TRV_BOTTOM,
                  {1.0f, 0.0f, 0.0f, 1.0f}, {0.8f, 0.8f, 0.0f, 1.0f});
        elapsedT = 0.0f;
        countedFrames = 0;
    }

    txt.updateCommandBuffer();
}

void SceneManager::cleanUp() {
    MSphere.cleanup();
    MCube.cleanup();
    MSoftbal.cleanup();
    MStatue.cleanup();
    Mplane.cleanup();

    for (int i = 0; i < 4; i++) {
        Talbedo[i].cleanup();
        TNorm[i].cleanup();
        Tmetal[i].cleanup();
        Troughness[i].cleanup();
        Tao[i].cleanup();
    }

    DSLlocal.cleanup();
    DSLglobal.cleanup();

    P.destroy();

    RP.destroy();

    txt.localCleanup();
}

void SceneManager::pipelinesAndDescriptorSetsCleanup() {
    P.cleanup();

    RP.cleanup();

    DSglobal.cleanup();
    DSlocalSphere.cleanup();
    DSlocalCube.cleanup();
    DSlocalSoftbal.cleanup();
    DSlocalStatue.cleanup();
    DSlocalPlane.cleanup();

    txt.pipelinesAndDescriptorSetsCleanup();
}
