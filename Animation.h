#pragma once
#include "Timer.h"

class Animation
{
public:
    Animation():
        m_timer(0), m_frame_count(0)
    {}

    Animation(int frame_count, float length):
        m_frame_count(frame_count), m_timer(length)
    {}

    [[nodiscard]] float get_length() const
    {
        return m_timer.get_length();
    }

    [[nodiscard]] int current_frame() const
    {
        return static_cast<int>(m_timer.get_time() / m_timer.get_length() * m_frame_count);
    }

    void step(float delta_time)
    {
        m_timer.step(delta_time);
    }

    bool is_done() const
    {
        return m_timer.is_time_out();
    }
private:
    Timer m_timer;
    int m_frame_count;
};
