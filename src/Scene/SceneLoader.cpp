#include "Scene/SceneLoader.hpp"

#include "CameraController.hpp"
#include "GameStatus.hpp"
#include "Player.hpp"
#include "uniform_buffers.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <utility>
#include <vector>

std::vector<PipelineAndTexturesDefs>
SceneLoader::generateTextureDef(std::string_view tech_name, std::vector<Pipeline> &pipelines, int numTextures) {
    std::vector<TextureDefs> textDefs;
    textDefs.reserve(numTextures);

    for (int i = 0; i < numTextures; i++) {
        textDefs.emplace_back(true, i, VkDescriptorImageInfo{});
    }

    if (tech_name == "General")
        textDefs.emplace_back(false, 0, VkDescriptorImageInfo{});

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

void SceneLoader::generateDescriptorSetLayout(nlohmann::json &technique, int numTextures) {
    std::string tech_name = technique.at("technique");

    if (dslMap.contains(tech_name))
        return;

    DescriptorSetLayout dsl{};
    if (tech_name == "General") {
        dsl.init(
            this->bp,
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1},
             {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GlobalUniformBufferObject), 1},
             {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures},
             {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5, 1}});

    } else if (tech_name == "Emissive") {
        dsl.init(
            this->bp,
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1},
             {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(LightUniformBufferObject), 1},
             {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures}});
    } else {
        dsl.init(this->bp,
                 {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1},
                  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, numTextures}});
    }
    dslMap.insert({std::move(tech_name), std::move(dsl)});
}

void SceneLoader::generatePipelines(nlohmann::json &jsonFile, VkDevice dev, VkPhysicalDevice physicalDev) {
    for (auto &technique : jsonFile.at("instances")) {
        std::string tech_name = technique.at("technique");
        int numTextures = technique.at("elements")[0].at("texture").size();
        ;

        if (dslMap.contains(tech_name))
            continue;

        generateDescriptorSetLayout(technique, numTextures);

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
            if (tech_name != "General") {
                P.setCullMode(VK_CULL_MODE_NONE);
            }
            if (tech_name == "Emissive") {
                P.setTransparency(true);
            }

            pipelines.push_back(std::move(P));
        }

        pMap.insert({tech_name, std::move(pipelines)});
        TechniqueRef pr{};
        pr.init(tech_name.c_str(), generateTextureDef(tech_name, pMap.at(tech_name), numTextures), numTextures, &vd);
        prs.push_back(pr);

        if (technique.contains("shadow")) {
            auto &shadow = technique.at("shadow");
            smMap[tech_name].init(this->bp, &vd, dev, physicalDev,
                                  {.vertShader = shadow.at("vertex"), .fragShader = shadow.at("fragment")});
        }
    }
}

void SceneLoader::generalUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController) {
    UniformBufferObject ubo{};
    GlobalUniformBufferObject gubo{};

    updatePlayerRoom(gameStatus);

    // dynamic light: only the torches of the player's room and of the rooms seen through open doors
    const std::vector<int> rooms = gameStatus.litRooms();
    auto isLit = [&](const Torch &t) {
        return gameStatus.playerRoom == 0 || std::ranges::find(rooms, t.room) != rooms.end();
    };

    std::vector<Torch *> validTs;
    for (auto &t : gameStatus.torches) {
        if (!gameStatus.settings.dynamicLight || (t.on && isLit(t))) {
            validTs.push_back(&t);
        }
    }

    if (gameStatus.settings.dynamicLight) {
        std::ranges::sort(validTs, [&gameStatus](auto *lhs, auto *rhs) -> bool {
            return glm::distance(gameStatus.player.pos, lhs->pos) < glm::distance(gameStatus.player.pos, rhs->pos);
        });
    }

    gubo.playerPos = glm::vec4(gameStatus.player.pos, 1.0f);
    gubo.settings.z = NUM_LIGHTS > validTs.size() ? validTs.size() : NUM_LIGHTS;

    for (int i = 0; i < gubo.settings.z; i++) {
        gubo.lightColor[i] = glm::vec4(validTs[i]->color, 1.0f);
        gubo.lightPos[i] = glm::vec4(validTs[i]->pos, 1.0f);
        gubo.lightParams[i] = glm::vec4(validTs[i]->on ? 1.0f : 0.0f, validTs[i]->g, validTs[i]->radius(),
                                        static_cast<float>(validTs[i] - gameStatus.torches.data()));
    }

    gubo.settings.x = gameStatus.settings.shadows ? 1.0f : 0.0f;
    gubo.settings.y = gameStatus.settings.shading;

    // Each instance is drawn with its own world matrix (from scene.json), the same one used by its collider
    for (auto *instance : techInstMap["General"]) {
        ubo.mMat = instance->Wm;
        ubo.mvpMat = cameraController.viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

        instance->DS[0][0]->map(currentImage, &ubo, 0);
        instance->DS[0][0]->map(currentImage, &gubo, 1);
    }
}

void SceneLoader::prepareShadowPass(int currentImage, GameStatus &gameStatus) {
    // a torch only draws the objects of its own room and of the rooms next to it
    std::vector<std::vector<int>> torchRooms;
    for (const auto &t : gameStatus.torches) {
        torchRooms.push_back(gameStatus.roomsAround(t.room, 1, false));
    }

    std::map<const Instance *, std::vector<int>> instanceRooms;
    for (const auto *inst : instances) {
        instanceRooms[inst] = roomsOf(*inst);
    }

    auto isCaster = [&](uint32_t light, const Instance &inst) {
        const std::string &id = *inst.id;
        // walls, roofs, doors and floor always block the light, or it would leak into the far rooms
        if (id.starts_with("Wall_") || id.starts_with("Roof_") || id.starts_with("door_") || id == "floor")
            return true;
        if (light >= torchRooms.size() || gameStatus.torches[light].room == 0)
            return true;

        return std::ranges::any_of(instanceRooms[&inst], [&](int room) {
            return std::ranges::find(torchRooms[light], room) != torchRooms[light].end();
        });
    };

    for (auto &sm : smMap | std::views::values) {
        sm.selectCasters(isCaster);

        // light k of the shadow maps is always torch k, whatever GUBO slot it takes
        std::vector<glm::vec4> lightPos(sm.getLightCount(), glm::vec4(0.0f, -1000.0f, 0.0f, 1.0f));
        for (std::size_t k = 0; k < lightPos.size() && k < gameStatus.torches.size(); k++) {
            lightPos[k] = glm::vec4(gameStatus.torches[k].pos, 1.0f);
        }
        sm.updateUniformBuffer(currentImage, lightPos);
    }
}

void SceneLoader::emissiveUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController) {
    UniformBufferObject ubo{};
    LightUniformBufferObject lubo{};

    // Each instance is drawn with its own world matrix (from scene.json), the same one used by its collider
    for (auto *instance : techInstMap["Emissive"]) {
        ubo.mMat = instance->Wm;
        ubo.mvpMat = cameraController.viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

        instance->DS[0][0]->map(currentImage, &ubo, 0);

        auto t_it =
            std::ranges::find_if(gameStatus.torches, [instance](const auto &t) { return *instance->id == t.id; });

        if (t_it != gameStatus.torches.end()) {
            lubo.lightColor = glm::vec4(t_it->color, t_it->on ? 1.0f : 0.0f);
            instance->DS[0][0]->map(currentImage, &lubo, 1);
        }
    }
}

void SceneLoader::armsUpdateUB(int currentImage, GameStatus &gameStatus, CameraController &cameraController) {
    UniformBufferObject ubo{};

    for (auto *instance : techInstMap["Arms"]) {
        ubo.mMat = cameraController.computeAttachedModelMat(gameStatus.player.pos, gameStatus.player.armsOffset);
        ubo.mvpMat = cameraController.viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));

        instance->DS[0][0]->map(currentImage, &ubo, 0);
    }
}

SceneLoader::SceneLoader(BaseProject *bp, std::string_view scene_json, VkDevice dev, VkPhysicalDevice physicalDev,
                         int w, int h)
    : bp(bp), w(w), h(h) {
    std::ifstream ifs(scene_json.data());
    nlohmann::json jsonFile = nlohmann::json::parse(ifs);

    generateVertexDescriptors(jsonFile);
    generatePipelines(jsonFile, dev, physicalDev);

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

    this->bp->DPSZs.uniformBlocksInPool += numElem * 5;
    this->bp->DPSZs.texturesInPool += numElem * 10;
    this->bp->DPSZs.setsInPool += numElem * 3;

    int nPasses = pMap.begin()->second.size();
    if (scene.init(this->bp, nPasses, vdrs, prs, scene_json.data()) != 0) {
        std::cout << "ERROR LOADING THE SCENE\n";
        exit(-1);
    }

    for (auto &sm : smMap | std::views::values) {
        sm.attachScene(&scene, [](const Instance &inst) -> bool { return !inst.id->starts_with("flame"); });
    }

    collisionDetector = std::make_unique<CollisionDetector>();
    for (int ti = 0; ti < scene.TechniqueInstanceCount; ti++) {
        for (int i = 0; i < scene.TI[ti].InstanceCount; i++) {
            auto *inst = &scene.TI[ti].I[i];
            instances.push_back(inst);
            techInstMap[*scene.TI[ti].T->id].push_back(inst);
            if (inst->C) {
                collisionDetector->addCollider(inst->C, !unsolid.contains(*inst->id));
            }
        }
    }

    computeRoomBounds();
    std::ranges::transform(instances, std::back_inserter(lastWm), [](const Instance *inst) { return inst->Wm; });

    rp.init(this->bp);
}

bool SceneLoader::geometryChanged() {
    bool changed = false;
    for (std::size_t i = 0; i < instances.size(); i++) {
        if (instances[i]->Wm != lastWm[i]) {
            lastWm[i] = instances[i]->Wm;
            changed = true;
        }
    }
    return changed;
}

std::vector<int> SceneLoader::roomsOf(const Instance &instance) const {
    const glm::vec3 p = instance.Wm[3];
    const AABBextents box = instance.C ? instance.C->getExtents() : AABBextents{p.x, p.x, p.y, p.y, p.z, p.z};
    constexpr float margin = 0.5f; // objects on a shared wall (doors) belong to both rooms

    std::vector<int> rooms;
    for (const auto &[room, b] : roomBounds) {
        if (box.xMin <= b.xMax + margin && box.xMax >= b.xMin - margin && box.yMin <= b.yMax + margin &&
            box.yMax >= b.yMin - margin && box.zMin <= b.zMax + margin && box.zMax >= b.zMin - margin) {
            rooms.push_back(room);
        }
    }
    return rooms;
}

void SceneLoader::computeRoomBounds() {
    for (auto *inst : instances) {
        const std::string &id = *inst->id;
        if (!inst->C || !(id.starts_with("Wall_") || id.starts_with("Roof_")))
            continue;

        const int room = std::atoi(id.c_str() + 5); // "Wall_<room>_..." / "Roof_<room>"
        const AABBextents e = inst->C->getExtents();
        auto [it, isNew] = roomBounds.try_emplace(room, e);
        if (isNew)
            continue;

        auto &b = it->second;
        b.xMin = std::min(b.xMin, e.xMin);
        b.xMax = std::max(b.xMax, e.xMax);
        b.yMin = std::min(b.yMin, e.yMin);
        b.yMax = std::max(b.yMax, e.yMax);
        b.zMin = std::min(b.zMin, e.zMin);
        b.zMax = std::max(b.zMax, e.zMax);
    }
}

void SceneLoader::updatePlayerRoom(GameStatus &gameStatus) const {
    const glm::vec3 &p = gameStatus.player.pos;
    auto inside = [&p](const AABBextents &b) {
        return p.x >= b.xMin && p.x <= b.xMax && p.z >= b.zMin && p.z <= b.zMax;
    };

    // rooms overlap at the shared walls: keep the current one while the player is still in it
    if (auto it = roomBounds.find(gameStatus.playerRoom); it != roomBounds.end() && inside(it->second))
        return;

    for (const auto &[room, bounds] : roomBounds) {
        if (inside(bounds)) {
            gameStatus.playerRoom = room;
            return;
        }
    }
}

void SceneLoader::onResize(int newW, int newH) {
    // the colliders' wireframes are drawn in their own render pass: it must have the same size as the main one
    rp.width = newW;
    rp.height = newH;
    scene.ColShow.resizeScreen(newW, newH);
}

void SceneLoader::pipelinesAndDescriptorSetsInit() {
    rp.create();
    for (auto &ps : pMap | std::views::values) {
        for (auto &p : ps) {
            p.create(&rp);
        }
    }

    // Not fully functional in case of different DSL
    for (auto &[tech_name, sm] : smMap) {
        sm.pipelinesAndDescriptorSetsInit();
        scene.TechniqueIds[tech_name]->PT[0].texDefs[0][5].info = sm.getViewAndSampler();
    }

    scene.pipelinesAndDescriptorSetsInit();
}

void SceneLoader::populateShadowCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    for (auto &sm : smMap | std::views::values) {
        sm.populateCommandBuffer(commandBuffer, currentImage);
    }
}

void SceneLoader::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    rp.begin(commandBuffer, currentImage);
    for (int i = 0; i < pMap.begin()->second.size(); i++) {
        scene.populateCommandBuffer(commandBuffer, i, currentImage);
    }
    rp.end(commandBuffer);
}

void SceneLoader::updateUniformBuffer(int currentImage, GameStatus &gameStatus, CameraController &cameraController) {
    generalUpdateUB(currentImage, gameStatus, cameraController);
    emissiveUpdateUB(currentImage, gameStatus, cameraController);
    armsUpdateUB(currentImage, gameStatus, cameraController);

    // updates the view-projection matrix used to draw the colliders' wireframes
    scene.updateColliderVisualizer(currentImage, cameraController.viewPrjMat);
}

const std::vector<Instance *> &SceneLoader::getInstances() { return instances; }

CollisionDetector *SceneLoader::getCollisionDetector() const { return collisionDetector.get(); }

void SceneLoader::refreshColliders() { scene.refreshColliderVisualizer(); }

void SceneLoader::cleanUp() {
    for (auto &dsl : dslMap | std::views::values) {
        dsl.cleanup();
    }

    for (auto &ps : pMap | std::views::values) {
        for (auto &p : ps) {
            p.destroy();
        }
    }

    for (auto &sm : smMap | std::views::values) {
        sm.localCleanup();
    }

    scene.localCleanup();
    rp.destroy();
}

void SceneLoader::pipelinesAndDescriptorSetsCleanup() {
    for (auto &ps : pMap | std::views::values) {
        for (auto &p : ps) {
            p.cleanup();
        }
    }

    for (auto &sm : smMap | std::views::values) {
        sm.pipelinesAndDescriptorSetsCleanup();
    }

    scene.pipelinesAndDescriptorSetsCleanup();
    rp.cleanup();
}
