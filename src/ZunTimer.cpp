#include "ZunTimer.hpp"
#include "Supervisor.hpp"

namespace th06 {

void ZunTimer::Initialize() {
    this->current = 0;
    this->previous = -1;
    this->subFrame = 0;
}

void ZunTimer::Increment(i32 value) {
    if (g_Supervisor.framerateMultiplier > 0.99f) {
        this->current = this->current + value;

        return;
    }

    if (value < 0) {
        Decrement(-value);

        return;
    }

    this->previous = this->current;
    this->subFrame = g_Supervisor.effectiveFramerateMultiplier * (float)value +
                     this->subFrame;

    while (this->subFrame >= 1.0f) {
        this->current++;
        this->subFrame = this->subFrame - 1.0f;
    }
}

void ZunTimer::Decrement(i32 value) {
    if (g_Supervisor.framerateMultiplier > 0.99f) {
        this->current = this->current - value;

        return;
    }

    if (value < 0) {
        Increment(-value);

        return;
    }

    this->previous = this->current;
    this->subFrame = this->subFrame -
                     g_Supervisor.effectiveFramerateMultiplier * (float)value;

    while (this->subFrame < 0.0f) {
        this->current--;
        this->subFrame = this->subFrame + 1.0f;
    }
}

i32 ZunTimer::NextTick() {
    this->Tick();
    return this->current;
}
}; // namespace th06
