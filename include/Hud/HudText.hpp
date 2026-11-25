#pragma once

#include "Hud/HudStructs.hpp"
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
    const float spacing = 0.7;

    void genElements();

  public:
    HudText(std::string_view jsonFile);
    std::vector<HudElement> &getElements();
    void addText(std::string_view id, Text text);
    void removeText(std::string_view id);
    bool isTextPresent(std::string_view id);
    void clean();
};
