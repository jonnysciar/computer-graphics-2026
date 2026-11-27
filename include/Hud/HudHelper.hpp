#pragma once

#include "InputManager.hpp"
#include "GameStatus.hpp"

#include <glm/fwd.hpp>
#include <array>
#include <string>

struct HudVertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

using HudQuad = std::array<HudVertex, 4>;

struct HudFrame {
    std::string name;
    glm::vec2 pos;
    glm::vec2 size;

    HudFrame() = default;
    HudFrame(nlohmann::json &json, int w, int h);

    HudQuad getCenteredQuad(float x, float y, float scale = 1.0f);
    HudQuad getQuad(float x, float y, float scale = 1.0f);
};

class HudElement : public Listener<GameStatus &, MouseState> {
  private:
    enum Type { NONE = 0, BUTTON, TOGGLE };

    Type type;

    void buttonOnEvent(GameStatus &gs, MouseState ms);
    void toggleOnEvent(GameStatus &gs, MouseState ms);

  public:
    std::vector<HudFrame *> frames{};
    int currentFrame{0};
    std::string name;
    glm::vec2 pos{0.0f, 0.0f};
    float scale{1.0f};
    bool visible{true};
    bool centered{false};
    bool interactable{false};

    bool dirty{false};

    std::function<void(HudElement *self, GameStatus &gs)> callback;

    HudElement() = default;
    HudElement(nlohmann::json &json, std::map<std::string, HudFrame> &frames);

    HudFrame *getCurrentFrame() const;
    HudQuad getQuad() const;
    bool isMouseHover(float x, float y);
    void onEvent(GameStatus &gs, MouseState ms) override;
};

namespace HudHelper {
void playResumeButtonHandler(HudElement *self, GameStatus &gs);
void settingsButtonHandler(HudElement *self, GameStatus &gs);
void exitButtonHandler(HudElement *self, GameStatus &gs);
void fpsToggleHandler(HudElement *self, GameStatus &gs);
void shadowsToggleHandler(HudElement *self, GameStatus &gs);
void dynamicLightToggleHandler(HudElement *self, GameStatus &gs);
void shadingToggleHandler(HudElement *self, GameStatus &gs);
} // namespace HudHelper
