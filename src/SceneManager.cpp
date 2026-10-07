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

    vdrs.resize(1);
    vdrs[0].init("VD", &VD);

    prs.resize(1);
    prs[0].init("HauntedCastle", {
                                        {&P, {
                                            {},
                                            {
                                                   //albedo
                                                {true, 0, {}},
                                                {true, 1, {}},
                                                {true, 2, {}},
                                                {true, 3, {}},
                                                   //normal
                                                {true, 4, {}},
                                                {true, 5, {}},
                                                {true, 6, {}},
                                                {true, 7, {}},
                                                   //metallic
                                                {true, 8, {}},
                                                {true, 9, {}},
                                                {true, 10, {}},
                                                {true, 11, {}},
                                                   //roughness
                                                {true, 12, {}},
                                                {true, 13, {}},
                                                {true, 14, {}},
                                                {true, 15, {}},
                                                   //ao
                                                {true, 16, {}},
                                                {true, 17, {}},
                                                {true, 18, {}},
                                                {true, 19, {}},
                                                }
                                        }}
                                        }, 20, &VD);

    // sets the size of the Descriptor Set Pool
    this->bp->DPSZs.uniformBlocksInPool = 20;
    this->bp->DPSZs.texturesInPool = 120;
    this->bp->DPSZs.setsInPool = 20;

    if(scene.init(this->bp, 1, vdrs, prs, scene_json.data()) != 0) {
        std::cout << "ERROR LOADING THE SCENE\n";
        exit(-1);
    }

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

    scene.pipelinesAndDescriptorSetsInit();
    txt.pipelinesAndDescriptorSetsInit();
}

void SceneManager::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    RP.begin(commandBuffer, currentImage);

    P.bind(commandBuffer);
    DSglobal.bind(commandBuffer, P, 0, currentImage);

    scene.populateCommandBuffer(commandBuffer, 0, currentImage);

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
    scene.TI[0].I[0].DS[0][0]->map(currentImage, &gubo, 0);
    scene.TI[0].I[0].DS[0][1]->map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(-6, 1, 0));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    scene.TI[0].I[1].DS[0][0]->map(currentImage, &gubo, 0);
    scene.TI[0].I[1].DS[0][1]->map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(6, 1, 0));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    scene.TI[0].I[2].DS[0][0]->map(currentImage, &gubo, 0);
    scene.TI[0].I[2].DS[0][1]->map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(-3, 1, -5));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    scene.TI[0].I[3].DS[0][0]->map(currentImage, &gubo, 0);
    scene.TI[0].I[3].DS[0][1]->map(currentImage, &ubo, 0);

    ubo.mMat = glm::translate(glm::mat4(1), glm::vec3(3, 0, -5)) * glm::scale(glm::mat4(1), glm::vec3(2.61));
    ubo.mvpMat = viewPrjMat * ubo.mMat;
    ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
    scene.TI[0].I[4].DS[0][0]->map(currentImage, &gubo, 0);
    scene.TI[0].I[4].DS[0][1]->map(currentImage, &ubo, 0);

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
    DSLlocal.cleanup();
    DSLglobal.cleanup();

    P.destroy();

    RP.destroy();

    scene.localCleanup();
    txt.localCleanup();
}

void SceneManager::pipelinesAndDescriptorSetsCleanup() {
    P.cleanup();

    RP.cleanup();

    DSglobal.cleanup();

    scene.pipelinesAndDescriptorSetsCleanup();
    txt.pipelinesAndDescriptorSetsCleanup();
}
