#pragma once

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

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

struct HudSprite {
    std::string name;
    glm::vec2 pos;
    glm::vec2 size;

    HudSprite(json::json &json, int w, int h) {
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
    HudSprite *sprite;
    glm::vec2 pos{0.0f, 0.0f};
    float scale{1.0f};
    bool visible{true};
    bool centered{false};
};
