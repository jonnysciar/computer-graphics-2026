#include "Hud/HudManager.hpp"

#include "GameStatus.hpp"
#include "modules/Starter.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <ranges>
#include <vulkan/vulkan_core.h>

void HudManager::populateCmdBufferAccess(VkCommandBuffer cmdBuf, int i, void *params) {
    auto *s = static_cast<HudManager::CmdBufStruct *>(params);
    s->hudManager->populateCommandBuffer(cmdBuf, i);
}

void HudManager::freeCmdBufferAccess(void *params) {
    auto *s = static_cast<HudManager::CmdBufStruct *>(params);
    if (s->mesh != nullptr) {
        s->mesh->cleanup();
        delete s->mesh;
    }
    delete s;
}

void HudManager::generateHudSprite(std::string_view jsonFile) {
    std::ifstream ifs(jsonFile.data());
    nlohmann::json json = nlohmann::json::parse(ifs);

    int w = json.at("meta").at("size").at("w").get<int>();
    int h = json.at("meta").at("size").at("h").get<int>();

    this->sprites.reserve(json.at("frames").size());
    for (auto &spriteJson : json.at("frames")) {
        this->sprites.emplace_back(spriteJson, w, h);
    }
}

void HudManager::buildMesh() {
    std::vector<HudVertex> verts;
    std::vector<uint32_t> idxs;

    for (const auto &elem : elements | std::views::values) {
        if (!elem.visible)
            continue;

        int baseVertex = verts.size();
        auto quadVerts = elem.centered ? elem.sprite->getCenteredQuad(elem.pos.x, elem.pos.y, elem.scale)
                                       : elem.sprite->getQuad(elem.pos.x, elem.pos.y, elem.scale);

        verts.insert(verts.end(), quadVerts.begin(), quadVerts.end());
        for (int i : {0u, 1u, 2u, 2u, 3u, 0u}) {
            idxs.push_back(baseVertex + i);
        }
    }

    if (verts.empty()) {
        m = nullptr;
        return;
    }

    m = new Model();
    m->vertices.resize(verts.size() * sizeof(HudVertex));
    std::memcpy(m->vertices.data(), verts.data(), m->vertices.size());
    m->indices = std::move(idxs);

    m->initMesh(bp, &vd, false);
}

HudManager::HudManager(BaseProject *basep, int w, int h) : bp(basep) {
    vd.init(bp, {{0, sizeof(HudVertex), VK_VERTEX_INPUT_RATE_VERTEX}},
            {{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(HudVertex, pos), sizeof(glm::vec3), UV},
             {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(HudVertex, uv), sizeof(glm::vec3), UV}});

    dsl.init(bp, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}});

    p.init(bp, &vd, "shaders/hud.vert.spv", "shaders/hud.frag.spv", {&dsl});
    p.setCullMode(VK_CULL_MODE_NONE);
    p.setTransparency(true);

    generateHudSprite("assets/textures/hud/hud.json");

    // Key 1
    elements["key_1"] = {&sprites[1], glm::vec2{-0.98f, -0.98f}, 0.5f, false};
    // Key 2
    elements["key_2"] = {&sprites[1], glm::vec2{-0.98f + sprites[1].size.x * 0.5f, -0.98f}, 0.5f, false};
    // Key 3
    elements["key_3"] = {&sprites[1], glm::vec2{-0.98f + 2.0f * sprites[1].size.x * 0.5f, -0.98f}, 0.5f, false};
    // Dialog
    elements["dialog"] = {&sprites[5], glm::vec2{0.0f, 0.8f}, 3.5f, false, true};
    // Menu
    elements["menu"] = {&sprites[0], glm::vec2{0.0f, 0.0f}, 2.5f, false, true};
    buildMesh();

    tex.init(bp, "assets/textures/hud/hud.png");

    AttachmentProperties color = (*RenderPass::getStandardAttchmentsProperties(AT_SURFACE_NOAA_DEPTH, bp))[0];
    color.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;             // keep the scene
    color.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // as left by the scene pass
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    std::vector<AttachmentProperties> props = {color};

    // Wait for the scene pass's color writes before loading/blending on top of them
    std::vector<VkSubpassDependency> deps = {
        {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0}};

    rp.init(bp, -1, -1, -1, &props, &deps);
}

void HudManager::onResize(int w, int h) {
    rp.width = w;
    rp.height = h;
}

void HudManager::pipelinesAndDescriptorSetsInit() {
    rp.create();
    p.create(&rp);
    ds.init(bp, &dsl, {tex.getViewAndSampler()});
}

void HudManager::populateCommandBuffer(VkCommandBuffer cmdBuf, int currentImage) {
    rp.begin(cmdBuf, currentImage);

    if (m != nullptr) {
        p.bind(cmdBuf);
        m->bind(cmdBuf);
        ds.bind(cmdBuf, p, 0, currentImage);
        vkCmdDrawIndexed(cmdBuf, m->indices.size(), 1, 0, 0, 0);
    }

    rp.end(cmdBuf);
}

void HudManager::updateUniformBuffer(int currentImage, GameStatus gameStatus, float deltaT) {
    if (elements["menu"].visible != gameStatus.settings.paused) {
        elements["menu"].visible = gameStatus.settings.paused;
        dirty = true;
    }

    for (const auto &id : gameStatus.player.collectedItems) {
        if (!elements[id].visible) {
            elements[id].visible = true;
            dirty = true;
        }
    }

    if (dirty) {
        buildMesh();
        auto *cmdBuf = new HudManager::CmdBufStruct();

        cmdBuf->hudManager = this;
        cmdBuf->mesh = this->m;

        bp->submitCommandBuffer("hud", 1, HudManager::populateCmdBufferAccess, cmdBuf, HudManager::freeCmdBufferAccess);
        dirty = false;
    }
}

void HudManager::setPrompt(std::string_view text) {
    elements["dialog"].visible = !text.empty();
    dirty = true;
}

void HudManager::setStatus(std::string_view text) {}

void HudManager::cleanUp() {
    if (m != nullptr) {
        m->cleanup();
        delete m;
    }
    tex.cleanup();
    dsl.cleanup();
    p.destroy();
    rp.destroy();
}

void HudManager::pipelinesAndDescriptorSetsCleanup() {
    ds.cleanup();
    p.cleanup();
    rp.cleanup();
}
