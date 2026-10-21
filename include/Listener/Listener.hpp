#pragma once

template <typename... Event> class Listener {
  public:
    virtual ~Listener() = default;
    virtual void onEvent(Event... e) = 0;

    Listener *getPtr() { return this; }
};