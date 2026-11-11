#pragma once

#include "SceneHandler.hpp"

class Room6Handler : public SceneHandler {
  public:
    Room6Handler(SceneLoader *sceneLoader, HUDManager *hudManager);
    ~Room6Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &gs, Key k) override;
};
