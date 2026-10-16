#pragma once

#include "Listener.hpp"

#include <algorithm>
#include <vector>

template <typename Event> class Listenable {
  private:
    std::vector<Listener<Event> *> listeners;

  public:
    virtual ~Listenable() = default;
    bool addListener(Listener<Event> *listener) {
        if (std::ranges::find(listeners, listener) != listeners.end()) {
            return false;
        }

        this->listeners.push_back(listener);
        return true;
    }

    void update(Player &p, Event event) {
        std::ranges::for_each(listeners, [&](Listener<Event> *listener) { listener->onEvent(p, event); });
    }
};