#pragma once

#include "Supervisor.hpp"
#include "inttypes.hpp"

namespace th06
{
struct ZunTimer
{
    i32 previous;
    f32 subFrame;
    i32 current;

    ZunTimer()
    {
        this->Initialize();
    }

    bool operator==(i32 time)
    {
        return this->current == time;
    }

    bool operator>=(i32 time)
    {
        return this->current >= time;
    }

    bool operator>(i32 time)
    {
        return this->current > time;
    }

    bool operator<(i32 time)
    {
        return this->current < time;
    }

    bool operator<=(i32 time)
    {
        return this->current <= time;
    }

    void Initialize();
    void Increment(i32 value);
    void Decrement(i32 value);
    i32 NextTick();

    void IncrementInline(i32 value)
    {
        this->Increment(value);
    }

    void InitializeForPopup()
    {
        this->current = 0;
        this->subFrame = 0;
        this->previous = -999;
    }

    void SetCurrent(i32 value)
    {
        this->current = value;
        this->subFrame = 0;
        this->previous = -999;
    }

    void Tick()
    {
        this->previous = this->current;
        g_Supervisor.TickTimer(&this->current, &this->subFrame);
    }

    f32 AsFramesFloat()
    {
        return this->current + this->subFrame;
    }

    i32 AsFrames()
    {
        return this->current;
    }

    bool HasTicked()
    {
        return this->current != this->previous;
    }
};
ZUN_ASSERT_SIZE(ZunTimer, 0xc);
}; // namespace th06
