#include "Hud/HudHelper.hpp"

HudFrame::HudFrame(nlohmann::json &json, int w, int h) {
    name = json.at("filename").get<std::string>();
    auto &frame = json.at("frame");

    pos.x = frame.at("x").get<float>() / w;
    pos.y = frame.at("y").get<float>() / h;

    size.x = frame.at("w").get<float>() / w;
    size.y = frame.at("h").get<float>() / h;
}

HudQuad HudFrame::getCenteredQuad(float x, float y, float scale) {
    glm::vec2 scaleSize = this->size * scale;
    glm::vec2 startPos = glm::vec2(x - scaleSize.x / 2.0f, y - scaleSize.y / 2.0f);
    return {HudVertex{.pos = startPos, .uv = this->pos},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y}, .uv = {this->pos.x + this->size.x, this->pos.y}},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y + scaleSize.y},
                      .uv = {this->pos.x + this->size.x, this->pos.y + this->size.y}},
            HudVertex{.pos = {startPos.x, startPos.y + scaleSize.y}, .uv = {this->pos.x, this->pos.y + this->size.y}}};
}

HudQuad HudFrame::getQuad(float x, float y, float scale) {
    glm::vec2 scaleSize = this->size * scale;
    glm::vec2 startPos = glm::vec2(x, y);
    return {HudVertex{.pos = startPos, .uv = this->pos},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y}, .uv = {this->pos.x + this->size.x, this->pos.y}},
            HudVertex{.pos = {startPos.x + scaleSize.x, startPos.y + scaleSize.y},
                      .uv = {this->pos.x + this->size.x, this->pos.y + this->size.y}},
            HudVertex{.pos = {startPos.x, startPos.y + scaleSize.y}, .uv = {this->pos.x, this->pos.y + this->size.y}}};
}

HudElement::HudElement(nlohmann::json &json, std::map<std::string, HudFrame> &frames) {

    currentFrame = json.at("currentFrame").get<int>();
    name = json.at("name").get<std::string>();
    for (const auto &f : json.at("frames")) {
        this->frames.push_back(&frames[f.get<std::string>()]);
    }
    pos.x = json.at("pos").at("x").get<float>();
    pos.y = json.at("pos").at("y").get<float>();
    scale = json.at("scale").get<float>();
    visible = json.at("visible").get<bool>();
    centered = json.at("centered").get<bool>();
    interactable = json.at("interactable").get<bool>();
}

HudFrame *HudElement::getCurrentFrame() const { return frames[currentFrame]; }

HudQuad HudElement::getQuad() const {
    return this->centered ? this->getCurrentFrame()->getCenteredQuad(this->pos.x, this->pos.y, this->scale)
                          : this->getCurrentFrame()->getQuad(this->pos.x, this->pos.y, this->scale);
}

bool HudElement::isMouseHover(float x, float y) {
    auto quad = getQuad();
    return (x > quad[0].pos.x && y > quad[0].pos.y) && (x < quad[2].pos.x && y < quad[2].pos.y);
}

void HudElement::onEvent(GameStatus &gs, MouseState ms) {
    if (!visible) {
        currentFrame = 0;
        return;
    }

    bool isHover = isMouseHover(ms.ndcX, ms.ndcY);

    if (!isHover) {
        dirty = currentFrame != 0;
        currentFrame = 0;
        return;
    }

    if (!ms.leftJustClicked) {
        dirty = currentFrame != 1;
        currentFrame = 1;
        return;
    }

    dirty = currentFrame != 2;
    currentFrame = 2;

    if (name == "z_play" || name == "z_resume") {
        gs.state = GameStatus::State::GAME;
        std::cout << "Set State to " << static_cast<int>(gs.state) << std::endl;
    }
}
