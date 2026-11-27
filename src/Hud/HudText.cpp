#include "Hud/HudText.hpp"
#include <glm/glm.hpp>
#include <ranges>
#include <string_view>
#include <fstream>

HudText::HudText(std::string_view jsonFile, glm::vec2 offset) {
    std::ifstream ifs(jsonFile.data());
    nlohmann::json json = nlohmann::json::parse(ifs);

    int w = json.at("meta").at("size").at("w").get<int>();
    int h = json.at("meta").at("size").at("h").get<int>();

    for (auto &frameJson : json.at("frames")) {
        auto &fjson = frameJson.at("frame");
        fjson.at("x") = fjson.at("x").get<float>() + offset.x;
        fjson.at("y") = fjson.at("y").get<float>() + offset.y;

        auto frame = HudFrame{frameJson, w, h};
        chars[frame.name.front()] = std::move(frame);
    }
}

void HudText::genElements() {
    elements.clear();
    for (const auto &text : texts | std::views::values) {
        elements.reserve(elements.size() + text.str.size());

        glm::vec2 pos = glm::vec2(text.params.x, text.params.y);
        float localSpacing = spacing * chars.begin()->second.size.y * text.params.z;

        if (text.params.w > 0.0f) {
            float w = computeTextLength(text.str, text.params.z);
            float h = chars.begin()->second.size.y * text.params.z;

            pos.x = pos.x - w / 2.0f;
            pos.y = pos.y - h / 2.0f;
        }

        for (int i = 0; i < text.str.size(); i++) {

            if (text.str[i] == ' ') {
                pos.x += localSpacing;
                continue;
            }
            auto *frame = &chars[text.str[i]];
            HudElement elem{};

            elem.frames = {frame};
            elem.pos = pos;
            elem.scale = text.params.z;

            elements.push_back(std::move(elem));

            pos.x += frame->size.x * text.params.z;
        }
    }
}

float HudText::computeTextLength(std::string_view text, float scale) {
    float size = 0.0f;
    for (const char c : text) {
        if (c == ' ') {
            size += chars.begin()->second.size.y * spacing;
            continue;
        }

        size += chars[c].size.x;
    }

    return size * scale;
}

std::vector<HudElement> &HudText::getElements() { return elements; }

void HudText::addText(std::string_view id, Text text) {
    texts[id] = std::move(text);
    genElements();
}

void HudText::removeText(std::string_view id) {
    texts.erase(id);
    genElements();
}

void HudText::removeAllTexts() {
    texts.clear();
    genElements();
}

bool HudText::isTextPresent(std::string_view id) { return texts.count(id) > 0; }
