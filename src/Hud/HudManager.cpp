#include "Hud/HudManager.hpp"

#include "GameStatus.hpp"
#include "Hud/HudHelper.hpp"
#include "modules/Starter.hpp"

#include <cstdio>
#include <glm/glm.hpp>

#include <array>
#include <fstream>
#include <memory>
#include <ranges>
#include <string>

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

HudManager::HudManager(BaseProject *basep, int w, int h) : bp(basep) {
    vd.init(bp, {{0, sizeof(HudVertex), VK_VERTEX_INPUT_RATE_VERTEX}},
            {{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(HudVertex, pos), sizeof(glm::vec3), UV},
             {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(HudVertex, uv), sizeof(glm::vec3), UV}});

    dsl.init(bp, {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}});

    p.init(bp, &vd, "shaders/hud.vert.spv", "shaders/hud.frag.spv", {&dsl});
    p.setCullMode(VK_CULL_MODE_NONE);
    p.setTransparency(true);

    std::ifstream ifs{"assets/textures/hud/hud_frames.json"};
    nlohmann::json hud_frames = nlohmann::json::parse(ifs);
    int texW = hud_frames.at("meta").at("size").at("w").get<int>();
    int texH = hud_frames.at("meta").at("size").at("h").get<int>();

    parseHudFrames(hud_frames, texW, texH);

    ifs = std::ifstream{"assets/textures/hud/hud_elements.json"};
    nlohmann::json hud_elements = nlohmann::json::parse(ifs);
    parseHudElements(hud_elements);

    elements["z_play"].callback = HudHelper::playResumeButtonHandler;
    elements["z_resume"].callback = HudHelper::playResumeButtonHandler;
    elements["z_settings"].callback = HudHelper::settingsButtonHandler;
    elements["z_exit"].callback = HudHelper::exitButtonHandler;

    elements["z_toggle_fps"].callback = HudHelper::fpsToggleHandler;
    elements["z_toggle_shadows"].callback = HudHelper::shadowsToggleHandler;
    elements["z_toggle_dyn_l"].callback = HudHelper::dynamicLightToggleHandler;
    elements["z_toggle_shading"].callback = HudHelper::shadingToggleHandler;

    hudText = std::make_unique<HudText>("assets/textures/hud/hud_font.json",
                                        this->frames["font"].pos * glm::vec2(texW, texH));

    tex.init(bp, "assets/textures/hud/hud_frames.png");

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
    bp->DPSZs.texturesInPool += 2;
    bp->DPSZs.setsInPool += 2;
}

void HudManager::parseHudFrames(nlohmann::json &json, int w, int h) {
    for (auto &frameJson : json.at("frames")) {
        auto frame = HudFrame{frameJson, w, h};
        this->frames[frame.name] = std::move(frame);
    }
}

void HudManager::parseHudElements(nlohmann::json &json) {
    for (auto &elemJson : json.at("elements")) {
        auto elem = HudElement{elemJson, this->frames};
        this->elements[elem.name] = std::move(elem);
    }
}

void HudManager::buildMesh() {
    std::vector<HudVertex> verts;
    std::vector<uint32_t> idxs;

    dirty = false;

    for (auto &elem : elements | std::views::values) {
        elem.dirty = false;
        if (!elem.visible)
            continue;

        int baseVertex = verts.size();
        auto quadVerts = elem.getQuad();

        verts.insert(verts.end(), quadVerts.begin(), quadVerts.end());
        for (int i : {0u, 1u, 2u, 2u, 3u, 0u}) {
            idxs.push_back(baseVertex + i);
        }
    }

    for (const auto &elem : hudText->getElements()) {
        if (!elem.visible)
            continue;

        int baseVertex = verts.size();
        auto quadVerts = elem.getQuad();

        verts.insert(verts.end(), quadVerts.begin(), quadVerts.end());
        for (int i : {0u, 1u, 2u, 2u, 3u, 0u}) {
            idxs.push_back(baseVertex + i);
        }
    }

    m = nullptr;
    if (!verts.empty()) {
        m = new Model();
        m->vertices.resize(verts.size() * sizeof(HudVertex));
        std::memcpy(m->vertices.data(), verts.data(), m->vertices.size());
        m->indices = std::move(idxs);

        m->initMesh(bp, &vd, false);
    }

    auto *cmdBuf = new HudManager::CmdBufStruct();
    cmdBuf->hudManager = this;
    cmdBuf->mesh = this->m;
    bp->submitCommandBuffer("hud", 1, HudManager::populateCmdBufferAccess, cmdBuf, HudManager::freeCmdBufferAccess);
}

void HudManager::buildFPS(GameStatus &gs, float deltaT) {
    static char str[15] = {0};

    if (gs.settings.showFps) {
        countedFrame++;
        elapsed += deltaT;
        if (elapsed > 1.0f) {
            float fps = (float)countedFrame / elapsed;
            elapsed = 0;
            countedFrame = 0;

            std::snprintf(str, 14, "FPS: %.1f", fps);
            hudText->addText("fps", {str, glm::vec4(0.79f, -0.95f, 3.0f, 1.0f)});

            dirty = true;
        }
        return;
    }

    if (!hudText->isTextPresent("fps"))
        return;

    elapsed = 0;
    countedFrame = 0;
    hudText->removeText("fps");
    dirty = true;
}

void HudManager::buildUI(GameStatus &gs) {
    static char str[50] = {0};
    if (gs.state == prevState) {
        return;
    }

    elements["menu"].visible = false;

    // Buttons
    elements["z_play"].visible = false;
    elements["z_settings"].visible = false;
    elements["z_exit"].visible = false;
    elements["z_resume"].visible = false;
    elements["z_back"].visible = false;

    // Toggles
    elements["z_toggle_fps"].visible = false;
    elements["z_toggle_shadows"].visible = false;
    elements["z_toggle_dyn_l"].visible = false;
    elements["z_toggle_shading"].visible = false;

    hudText->removeText("menu");
    hudText->removeText("settings_fps");
    hudText->removeText("settings_shadows");
    hudText->removeText("settings_dyn_l");
    hudText->removeText("settings_shading");
    dirty = true;

    switch (gs.state) {
    case GameStatus::State::START:
        elements["menu"].visible = true;
        elements["z_play"].visible = true;
        elements["z_settings"].visible = true;
        elements["z_exit"].visible = true;
        hudText->addText("menu", {"Welcome to Haunted Castle!", glm::vec4(0.0f, -0.45f, 4.0f, 1.0f)});
        break;
    case GameStatus::State::GAME:
        break;
    case GameStatus::State::PAUSE:
        elements["menu"].visible = true;
        elements["z_resume"].visible = true;
        elements["z_settings"].visible = true;
        elements["z_exit"].visible = true;
        hudText->addText("menu", {"Game is paused", glm::vec4(0.0f, -0.45f, 5.0f, 1.0f)});
        break;
    case GameStatus::State::SETTINGS:
        elements["menu"].visible = true;
        elements["z_toggle_fps"].visible = true;
        elements["z_toggle_shadows"].visible = true;
        elements["z_toggle_dyn_l"].visible = true;
        elements["z_toggle_shading"].visible = true;
        elements["z_back"].visible = true;
        elements["z_back"].callback = [prevState = this->prevState](HudElement *self, GameStatus &gs) {
            gs.state = prevState;
        };

        elements["z_toggle_fps"].currentFrame = gs.settings.showFps;
        elements["z_toggle_shadows"].currentFrame = gs.settings.shadows;
        elements["z_toggle_dyn_l"].currentFrame = gs.settings.dynamicLight;
        elements["z_toggle_shading"].currentFrame = gs.settings.shading;

        std::snprintf(str, 49, "Change shading (%s)", gs.settings.shadingString().data());

        hudText->addText("menu", {"Settings", glm::vec4(0.0f, -0.45f, 5.0f, 1.0f)});
        hudText->addText("settings_fps", {"Enable FPS counter   ", glm::vec4(-0.8f, -0.25f, 3.0f, 0.0f)});
        hudText->addText("settings_shadows", {"Enable Shadows       ", glm::vec4(-0.8f, -0.05f, 3.0f, 0.0f)});
        hudText->addText("settings_dyn_l", {"Enable dynamic lights", glm::vec4(-0.8f, 0.15f, 3.0f, 0.0f)});
        hudText->addText("settings_shading", {str, glm::vec4(-0.8f, 0.35f, 2.3f, 0.0f)});
        break;
    case GameStatus::State::END:
        hudText->removeAllTexts();
        elements["menu"].visible = true;
        elements["prompt"].visible = false;
        elements["z_exit"].visible = true;
        hudText->addText("menu", {"CONGRATULATIONS!", glm::vec4(0.0f, -0.45f, 5.0f, 1.0f)});
        hudText->addText("menu_2", {"You completed the ", glm::vec4(0.0f, -0.15f, 4.0f, 1.0f)});
        hudText->addText("menu_3", {"Hunted Castle Game!", glm::vec4(0.0f, 0.0f, 5.0f, 1.0f)});
        break;
    default:
        break;
    }

    prevState = gs.state;
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

void HudManager::updateUniformBuffer(int currentImage, GameStatus &gameStatus, float deltaT) {
    buildFPS(gameStatus, deltaT);
    buildUI(gameStatus);

    for (const auto &id : gameStatus.player.collectedItems) {
        if (!elements[id].visible) {
            elements[id].visible = true;
            dirty |= true;
        }
    }

    for (auto &elem : elements | std::views::values) {
        dirty |= elem.dirty;
    }

    if (dirty) {
        buildMesh();
    }
}

void HudManager::setPrompt(std::string_view text) {
    elements["prompt"].visible = !text.empty();
    if (text.empty()) {
        hudText->removeText("prompt");
    } else {
        hudText->addText("prompt",
                         {text, glm::vec4(elements["prompt"].pos, 3.0f, elements["prompt"].centered ? 1.0f : 0.0f)});
    }

    dirty = true;
}

void HudManager::subscribeElements(Listenable<GameStatus &, MouseState> &listenable) {
    for (auto &elem : elements | std::views::values) {
        if (!elem.interactable)
            continue;

        listenable.addListener(elem.getPtr());
    }
}

void HudManager::cleanUp() {
    if (m != nullptr) {
        m->cleanup();
    }
    tex.cleanup();
    dsl.cleanup();
    p.destroy();
    rp.destroy();
}

void HudManager::pipelinesAndDescriptorSetsCleanup() {
    p.cleanup();
    rp.cleanup();
    ds.cleanup();
}
