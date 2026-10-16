#pragma once

template <typename Event> class Listener {
  public:
    virtual ~Listener() = default;
    virtual void onEvent(Player &p, Event event) = 0;
};