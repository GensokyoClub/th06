#pragma once
#include "Supervisor.hpp"

#include "ZunResult.hpp"
#include "decomp.hpp"

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

    BOOL operator==(i32 time)
    {
        return this->current == time;
    }

    BOOL operator>=(i32 time)
    {
        return this->current >= time;
    }

    BOOL operator>(i32 time)
    {
        return this->current > time;
    }

    BOOL operator<(i32 time)
    {
        return this->current < time;
    }

    BOOL operator<=(i32 time)
    {
        return this->current <= time;
    }

    i32 operator%(i32 divisor)
    {
        return this->current % divisor;
    }

    void Initialize();
    void Increment(i32 value);
    void Decrement(i32 value);

    i32 Tick()
    {
        this->previous = this->current;
        g_Supervisor.TickTimer(&this->current, &this->subFrame);
        return this->current;
    }

    void operator+=(i32 value)
    {
        this->Increment(value);
    }
    void operator-=(i32 value)
    {
        this->Decrement(value);
    }
    void operator++(int)
    {
        this->Tick();
    }
    void operator--(int)
    {
        this->Decrement(1);
    }

    void SetCurrent(i32 value)
    {
        this->current = value;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    void operator=(i32 value)
    {
        SetCurrent(value);
    }

    operator f32()
    {
        return this->current + this->subFrame;
    }

    operator i32()
    {
        return this->current;
    }

    BOOL HasTicked()
    {
        return this->current != this->previous;
    }
};
ZUN_ASSERT_TYPE(ZunTimer, 0xc, 4);
} // namespace th06
