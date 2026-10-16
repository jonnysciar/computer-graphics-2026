#include "SceneLoader.hpp"

#include "structs.hpp"

std::vector<PipelineAndTexturesDefs> SceneLoader::generateTextureDef(Pipeline *P, int numTextures) {
    std::vector<TextureDefs> textDefs;
    textDefs.reserve(numTextures);

    for (int i = 0; i < numTextures; i++) {
        textDefs.emplace_back(true, i, VkDescriptorImageInfo{});
    }

    return {
        {
            P, std::vector{
                std::vector<TextureDefs>{}, std::move(textDefs)
            }
        }
    };
}

void SceneLoader::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
    auto *SM = static_cast<SceneLoader *>(Params);
    SM->populateCommandBuffer(commandBuffer, currentImage);
}

SceneLoader::SceneLoader(BaseProject *bp, std::string_view scene_json, std::string_view vert_shader,
                         std::string_view frag_shader, int numTextures, int w, int h, int guboSize,
                         int uboSize) : bp(bp), w(w), h(h) {
    DSLlocal.init(this->bp, {
                      {
                          0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,
                          uboSize, 1
                      },
                      {
                          1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                          VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures
                      }
                  });
    DSLglobal.init(this->bp, {
                       {
                           0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS,
                           guboSize, 1
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
    P.init(this->bp, &VD, vert_shader.data(), frag_shader.data(), {&DSLglobal, &DSLlocal});

    std::ifstream ifs(scene_json.data());
    nlohmann::json jsonFile = nlohmann::json::parse(ifs);

    std::string vdName = jsonFile.at("models").at(0).at("VD");
    std::string instName = jsonFile.at("instances").at(0).at("technique");
    int numElem = jsonFile.at("instances").at(0).at("elements").size();

    vdrs.resize(1);
    vdrs[0].init(vdName.c_str(), &VD);

    prs.resize(1);
    prs[0].init(instName.c_str(), generateTextureDef(&P, numTextures), numTextures, &VD);

    // sets the size of the Descriptor Set Pool
    this->bp->DPSZs.uniformBlocksInPool += numElem * 2;
    this->bp->DPSZs.texturesInPool += numElem * numTextures;
    this->bp->DPSZs.setsInPool += numElem * 2;

    if (scene.init(this->bp, 1, vdrs, prs, scene_json.data()) != 0) {
        std::cout << "ERROR LOADING THE SCENE\n";
        exit(-1);
    }

    // submits the main command buffer
    bp->submitCommandBuffer("main", 0, populateCommandBufferAccess, this);
}

void SceneLoader::onResize(int newW, int newH) {
    RP.width = newW;
    RP.height = newH;
    // the colliders' wireframes are drawn in their own render pass: it must have the same size as the main one
    scene.ColShow.resizeScreen(newW, newH);
}

void SceneLoader::pipelinesAndDescriptorSetsInit() {
    RP.create();

    P.create(&RP);

    scene.pipelinesAndDescriptorSetsInit();
}

void SceneLoader::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    RP.begin(commandBuffer, currentImage);

    P.bind(commandBuffer);

    scene.populateCommandBuffer(commandBuffer, 0, currentImage);

    RP.end(commandBuffer);
}

void SceneLoader::updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat) {
    // updates the view-projection matrix used to draw the colliders' wireframes
    scene.updateColliderVisualizer(currentImage, viewPrjMat);
}

std::vector<Instance *> SceneLoader::getInstances() const {
    std::vector<Instance *> instances(scene.TI[0].InstanceCount);

    for (int i = 0; i < scene.TI[0].InstanceCount; i++) {
        instances[i] = &scene.TI[0].I[i];
    }

    return instances;
}

void SceneLoader::refreshColliders() {
    scene.refreshColliderVisualizer();
}

void SceneLoader::cleanUp() {
    DSLlocal.cleanup();
    DSLglobal.cleanup();

    P.destroy();

    RP.destroy();

    scene.localCleanup();
}

void SceneLoader::pipelinesAndDescriptorSetsCleanup() {
    P.cleanup();

    RP.cleanup();

    scene.pipelinesAndDescriptorSetsCleanup();
}

