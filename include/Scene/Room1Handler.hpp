#pragma once

#include "SceneHandler.hpp"

class Room1Handler : public SceneHandler {
  public:
    Room1Handler(SceneLoader *sceneLoader);
    ~Room1Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &gs, Key k) override;
};
