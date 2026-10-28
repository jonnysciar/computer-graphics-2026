#pragma once

#include "SceneHandler.hpp"

class Room2Handler : public SceneHandler {
  public:
    Room2Handler(SceneLoader *sceneLoader);
    ~Room2Handler() override = default;

    void onInteractionReach(std::string_view id, GameStatus &gs) override;

    void offInteractionReach(std::string_view id, GameStatus &gs) override;

    void onEvent(GameStatus &p, Key k) override;
};
