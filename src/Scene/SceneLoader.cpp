#include "Scene/SceneLoader.hpp"

#include "structs.hpp"

#include <unordered_set>

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

void SceneLoader::generateDescriptorSetLayout(nlohmann::json &technique, std::size_t guboSize, std::size_t uboSize,
                                              int numTextures) {
    std::string tech_name = technique.at("technique");

    if (dslMap.contains(tech_name))
        return;

    DescriptorSetLayout dsl{};
    dsl.init(this->bp,
             {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, static_cast<int>(uboSize), 1},
              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<int>(guboSize), 1},
              {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures}});
    dslMap.insert({std::move(tech_name), std::move(dsl)});
}

void SceneLoader::generatePipelines(nlohmann::json &jsonFile, std::size_t guboSize, std::size_t uboSize) {
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

SceneLoader::SceneLoader(BaseProject *bp, std::string_view scene_json, int w, int h, std::size_t guboSize,
                         std::size_t uboSize, bool clearScreen)
    : bp(bp), w(w), h(h) {
    std::ifstream ifs(scene_json.data());
    nlohmann::json jsonFile = nlohmann::json::parse(ifs);

    generateVertexDescriptors(jsonFile);
    generatePipelines(jsonFile, guboSize, uboSize);

    int numElem = 0;
    std::unordered_set<std::string> unsolid;
    for (const auto &tech : jsonFile.at("instances")) {
        auto &elements = tech.at("elements");
        numElem += elements.size();
        for (const auto &elem : elements) {
            if (elem.contains("solid") && !elem.at("solid")) {
                unsolid.insert(elem.at("id"));
            }
        }
    }

    this->bp->DPSZs.uniformBlocksInPool += numElem * 2;
    this->bp->DPSZs.texturesInPool += numElem * 5;
    this->bp->DPSZs.setsInPool += numElem;

    int nPasses = pMap.begin()->second.size();
    if (scene.init(this->bp, nPasses, vdrs, prs, scene_json.data()) != 0) {
        std::cout << "ERROR LOADING THE SCENE\n";
        exit(-1);
    }

    collisionDetector = std::make_unique<CollisionDetector>();
    for (int ti = 0; ti < scene.TechniqueInstanceCount; ti++) {
        for (int i = 0; i < scene.TI[ti].InstanceCount; i++) {
            auto *inst = &scene.TI[ti].I[i];
            instances.push_back(inst);
            collisionDetector->addCollider(inst->C, true);
        }
    }
}

void SceneLoader::onResize(int newW, int newH) {
    // the colliders' wireframes are drawn in their own render pass: it must have the same size as the main one
    scene.ColShow.resizeScreen(newW, newH);
}

void SceneLoader::pipelinesAndDescriptorSetsInit(RenderPass &rp) {
    for (auto &[key, ps] : pMap) {
        int i = 0;
        for (auto &p : ps) {
            p.create(&rp);
            i++;
        }
    }

    scene.pipelinesAndDescriptorSetsInit();
}

void SceneLoader::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    // TODO if shadow mapping probably rework
    for (int i = 0; i < pMap.begin()->second.size(); i++) {
        scene.populateCommandBuffer(commandBuffer, i, currentImage);
    }
}

void SceneLoader::updateUniformBuffer(uint32_t currentImage, glm::mat4 viewPrjMat) {
    // updates the view-projection matrix used to draw the colliders' wireframes
    scene.updateColliderVisualizer(currentImage, viewPrjMat);
}

const std::vector<Instance *> &SceneLoader::getInstances() { return instances; }

CollisionDetector *SceneLoader::getCollisionDetector() const { return collisionDetector.get(); }

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

    scene.localCleanup();
}

void SceneLoader::pipelinesAndDescriptorSetsCleanup() {
    for (auto &[key, ps] : pMap) {
        for (auto &p : ps) {
            p.cleanup();
        }
    }

    scene.pipelinesAndDescriptorSetsCleanup();
}
