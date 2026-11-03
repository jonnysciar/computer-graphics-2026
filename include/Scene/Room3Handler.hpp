#pragma once

#include "SceneHandler.hpp"

class Room3Handler : public SceneHandler {
  public:
    Room3Handler(SceneLoader *sceneLoader, HUDManager *hudManager);
    ~Room3Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &gs, Key k) override;
};
