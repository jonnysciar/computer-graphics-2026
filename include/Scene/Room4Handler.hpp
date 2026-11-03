#pragma once

#include "SceneHandler.hpp"

class Room4Handler : public SceneHandler {
  public:
    Room4Handler(SceneLoader *sceneLoader, HUDManager *hudManager);
    ~Room4Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &gs, Key k) override;
};
