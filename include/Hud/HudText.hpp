#pragma once

#include "Hud/HudHelper.hpp"
#include <glm/fwd.hpp>
#include <string_view>
#include <vector>

class HudText {
  public:
    struct Text {
        std::string_view str;
        // params: x,y position, z scale, w centered
        glm::vec4 params;
    };

  private:
    std::map<char, HudFrame> chars;
    std::vector<HudElement> elements;
    std::map<std::string_view, Text> texts;
    const float spacing = 0.3;

    void genElements();
    float computeTextLength(std::string_view text, float scale);

  public:
    HudText(std::string_view jsonFile, glm::vec2 offset = glm::vec2{0.0f});
    std::vector<HudElement> &getElements();
    void addText(std::string_view id, Text text);
    void removeText(std::string_view id);
    void removeAllTexts();
    bool isTextPresent(std::string_view id);
};
