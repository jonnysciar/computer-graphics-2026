#pragma once

#include "SceneHandler.hpp"

class Room7Handler : public SceneHandler {
  public:
    Room7Handler(SceneLoader *sceneLoader, HudManager *hudManager, GameStatus &gs);
    ~Room7Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &gs, Key k) override;
};
