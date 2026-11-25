#pragma once

#include "nlohmann/json.hpp"

#include <array>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <string>

namespace json = nlohmann;

struct HudVertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

struct HudFrame {
    std::string name;
    glm::vec2 pos;
    glm::vec2 size;

    HudFrame() = default;

    HudFrame(json::json &json, int w, int h) {
        name = json.at("filename").get<std::string>();
        auto &frame = json.at("frame");

        pos.x = frame.at("x").get<float>() / w;
        pos.y = frame.at("y").get<float>() / h;

        size.x = frame.at("w").get<float>() / w;
        size.y = frame.at("h").get<float>() / h;
    }

    std::array<HudVertex, 4> getCenteredQuad(float x, float y, float scale = 1.0f) {
        glm::vec2 scaleSize = this->size * scale;
        glm::vec2 startPos = glm::vec2(x - scaleSize.x / 2.0f, y - scaleSize.y / 2.0f);
        return {
            HudVertex{.pos = startPos, .uv = this->pos},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y}, .uv = {this->pos.x + this->size.x, this->pos.y}},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y + scaleSize.y},
                      .uv = {this->pos.x + this->size.x, this->pos.y + this->size.y}},
            HudVertex{.pos = {startPos.x, startPos.y + scaleSize.y}, .uv = {this->pos.x, this->pos.y + this->size.y}}};
    }

    std::array<HudVertex, 4> getQuad(float x, float y, float scale = 1.0f) {
        glm::vec2 scaleSize = this->size * scale;
        glm::vec2 startPos = glm::vec2(x, y);
        return {
            HudVertex{.pos = startPos, .uv = this->pos},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y}, .uv = {this->pos.x + this->size.x, this->pos.y}},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y + scaleSize.y},
                      .uv = {this->pos.x + this->size.x, this->pos.y + this->size.y}},
            HudVertex{.pos = {startPos.x, startPos.y + scaleSize.y}, .uv = {this->pos.x, this->pos.y + this->size.y}}};
    }
};

struct HudElement {
    HudFrame *sprite;
    std::string name;
    glm::vec2 pos{0.0f, 0.0f};
    float scale{1.0f};
    bool visible{true};
    bool centered{false};

    HudElement() = default;

    HudElement(json::json &json, std::map<std::string, HudFrame> &frames) {
        sprite = &frames[json.at("frame").get<std::string>()];
        name = json.at("name").get<std::string>();
        pos.x = json.at("pos").at("x").get<float>();
        pos.y = json.at("pos").at("y").get<float>();
        scale = json.at("scale").get<float>();
        visible = json.at("visible").get<bool>();
        centered = json.at("centered").get<bool>();
    }
};
