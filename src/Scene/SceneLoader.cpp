#include "Scene/SceneLoader.hpp"

#include "structs.hpp"

std::vector<PipelineAndTexturesDefs> SceneLoader::generateTextureDef(std::vector<Pipeline> &pipelines,
                                                                     int numTextures) {
    std::vector<TextureDefs> textDefs;
    textDefs.reserve(numTextures);

    for (int i = 0; i < numTextures; i++) {
        textDefs.emplace_back(true, i, VkDescriptorImageInfo{});
    }

    std::vector<PipelineAndTexturesDefs> pts;
    pts.reserve(pipelines.size());
    for (auto &pipeline : pipelines) {
        pts.emplace_back(&pipeline, std::vector<std::vector<TextureDefs>>{textDefs});
    }

    return pts;
}

void SceneLoader::generateVertexDescriptors(nlohmann::json &jsonFile) {
    for (auto &model : jsonFile.at("models")) {
        std::string vd_name = model.at("VD");

        if (vdMap.contains(vd_name))
            continue;

        VertexDescriptor vd{};
        vd.init(this->bp, {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
                {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos), sizeof(glm::vec3), POSITION},
                 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm), sizeof(glm::vec3), NORMAL},
                 {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV), sizeof(glm::vec2), UV},
                 {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tan), sizeof(glm::vec4), TANGENT}});
        vdMap.insert({vd_name, std::move(vd)});
        VertexDescriptorRef vdr{};
        vdr.init(vd_name.c_str(), &vdMap.at(vd_name));
        vdrs.push_back(vdr);
    }
}

void SceneLoader::generateDescriptorSetLayout(nlohmann::json &technique, int guboSize, int uboSize, int numTextures) {
    std::string tech_name = technique.at("technique");

    if (dslMap.contains(tech_name))
        return;

    DescriptorSetLayout dsl{};
    dsl.init(this->bp, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, uboSize, 1},
                        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, guboSize, 1},
                        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures}});
    dslMap.insert({std::move(tech_name), std::move(dsl)});
}

void SceneLoader::generatePipelines(nlohmann::json &jsonFile, int guboSize, int uboSize) {
    for (auto &technique : jsonFile.at("instances")) {
        std::string tech_name = technique.at("technique");
        int numTextures = technique.at("elements")[0].at("texture").size();
        ;

        if (dslMap.contains(tech_name))
            continue;

        generateDescriptorSetLayout(technique, guboSize, uboSize, numTextures);

        std::string model_name = technique.at("elements")[0].at("model");
        std::string vd_name;

        for (const auto &model : jsonFile.at("models")) {
            if (model.at("id") == model_name) {
                vd_name = model.at("VD");
                break;
            }
        }

        if (vd_name.empty()) {
            std::cout << "ERROR FINDING VD\n";
            std::exit(EXIT_FAILURE);
        }

        auto &vd = vdMap.at(vd_name);

        std::vector<Pipeline> pipelines;
        for (auto &pipeline : technique.at("pipelines")) {
            std::string vert_shader = pipeline.at("vertex");
            std::string frag_shader = pipeline.at("fragment");
            Pipeline P{};
            P.init(this->bp, &vd, vert_shader, frag_shader, {&dslMap.at(tech_name)});

            pipelines.push_back(std::move(P));
        }

        pMap.insert({tech_name, std::move(pipelines)});
        TechniqueRef pr{};
        pr.init(tech_name.c_str(), generateTextureDef(pMap.at(tech_name), numTextures), numTextures, &vd);
        prs.push_back(pr);
    }
}

void SceneLoader::populateCommandBufferAccess(VkCommandBuffer commandBuffer, int currentImage, void *Params) {
    auto *SM = static_cast<SceneLoader *>(Params);
    SM->populateCommandBuffer(commandBuffer, currentImage);
}

SceneLoader::SceneLoader(BaseProject *bp, std::string_view scene_json, int w, int h, int guboSize, int uboSize)
    : bp(bp), w(w), h(h) {
    std::ifstream ifs(scene_json.data());
    nlohmann::json jsonFile = nlohmann::json::parse(ifs);

    generateVertexDescriptors(jsonFile);
    generatePipelines(jsonFile, guboSize, uboSize);

    int nPasses = pMap.begin()->second.size();

    rps.resize(nPasses);
    for (auto &rp : rps) {
        rp.init(this->bp);
    }

    int numElem = 0;
    for (const auto &tech : jsonFile.at("instances")) {
        numElem += tech.at("elements").size();
    }

    this->bp->DPSZs.uniformBlocksInPool += numElem * 2;
    this->bp->DPSZs.texturesInPool += numElem * 5;
    this->bp->DPSZs.setsInPool += numElem;

    if (scene.init(this->bp, nPasses, vdrs, prs, scene_json.data()) != 0) {
        std::cout << "ERROR LOADING THE SCENE\n";
        exit(-1);
    }

    // submits the main command buffer
    this->bp->submitCommandBuffer("main", 0, populateCommandBufferAccess, this);
}

void SceneLoader::onResize(int newW, int newH) {
    for (auto &rp : rps) {
        rp.width = newW;
        rp.height = newH;
    }
    // the colliders' wireframes are drawn in their own render pass: it must have the same size as the main one
    scene.ColShow.resizeScreen(newW, newH);
}

void SceneLoader::pipelinesAndDescriptorSetsInit() {
    for (auto &rp : rps) {
        rp.create();
    }

    for (auto &[key, ps] : pMap) {
        int i = 0;
        for (auto &p : ps) {
            p.create(&rps[i]);
            i++;
        }
    }

    scene.pipelinesAndDescriptorSetsInit();
}

void SceneLoader::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    for (auto &rp : rps) {
        rp.begin(commandBuffer, currentImage);
    }

    for (auto &[key, ps] : pMap) {
        for (auto &p : ps) {
            p.bind(commandBuffer);
        }
    }

    scene.populateCommandBuffer(commandBuffer, 0, currentImage);

    for (auto &rp : rps) {
        rp.end(commandBuffer);
    }
}

void SceneLoader::updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat) {
    // updates the view-projection matrix used to draw the colliders' wireframes
    scene.updateColliderVisualizer(currentImage, viewPrjMat);
}

const std::vector<Instance *> &SceneLoader::getInstances() {
    if (!instances.empty())
        return instances;

    for (int ti = 0; ti < scene.TechniqueInstanceCount; ti++) {
        for (int i = 0; i < scene.TI[ti].InstanceCount; i++) {
            instances.push_back(&scene.TI[ti].I[i]);
        }
    }

    return instances;
}

void SceneLoader::refreshColliders() { scene.refreshColliderVisualizer(); }

void SceneLoader::cleanUp() {
    for (auto &dsl : dslMap | std::views::values) {
        dsl.cleanup();
    }

    for (auto &[key, ps] : pMap) {
        for (auto &p : ps) {
            p.destroy();
        }
    }

    for (auto &rp : rps) {
        rp.destroy();
    }

    scene.localCleanup();
}

void SceneLoader::pipelinesAndDescriptorSetsCleanup() {
    for (auto &[key, ps] : pMap) {
        for (auto &p : ps) {
            p.cleanup();
        }
    }

    for (auto &rp : rps) {
        rp.cleanup();
    }

    scene.pipelinesAndDescriptorSetsCleanup();
}
