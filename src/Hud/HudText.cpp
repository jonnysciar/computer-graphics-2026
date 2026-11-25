#include "Hud/HudText.hpp"
#include "Hud/HudStructs.hpp"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <ranges>
#include <string_view>
#include <fstream>

HudText::HudText(std::string_view jsonFile) {
    std::ifstream ifs(jsonFile.data());
    nlohmann::json json = nlohmann::json::parse(ifs);

    int w = json.at("meta").at("size").at("w").get<int>();
    int h = json.at("meta").at("size").at("h").get<int>();

    for (auto &frameJson : json.at("frames")) {
        auto frame = HudFrame{frameJson, w, h};
        chars[frame.name.front()] = std::move(frame);
    }
}

void HudText::genElements() {
    clean();
    for (const auto &text : texts | std::views::values) {
        elements.reserve(elements.size() + text.str.size());

        glm::vec2 startPos = glm::vec2(text.params.x, text.params.y);
        const HudFrame &chFrame = chars.begin()->second;
        float localSpacing = chFrame.size.x * text.params.z * spacing;

        if (text.params.w > 0.0f) {
            float w = localSpacing * text.str.size();
            float h = chFrame.size.y * text.params.z;

            startPos.x = startPos.x - w / 2.0f;
            startPos.y = startPos.y - h / 2.0f;
        }

        for (int i = 0; i < text.str.size(); i++) {
            if (text.str[i] == ' ')
                continue;

            glm::vec2 pos{startPos.x + static_cast<float>(i) * localSpacing, startPos.y};
            HudElement elem{};
            elem.sprite = &chars[text.str[i]], elem.name = {}, elem.pos = pos, elem.scale = text.params.z,
            elem.visible = true, elem.centered = false,

            elements.push_back(std::move(elem));
        }
    }
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

bool HudText::isTextPresent(std::string_view id) { return texts.count(id) > 0; }

void HudText::clean() { elements.clear(); }
