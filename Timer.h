#pragma once
class Timer
{
public:
    Timer(float length)
        : m_length(length), m_time(0), m_time_up(false) {};

    void step(float delta_time)
    {
        m_time += delta_time;
        if (m_time >= m_length)
        {
            m_time -= m_length;
            m_time_up = true;
        }
    }

    [[nodiscard]] bool is_time_out() const
    {
        return m_time_up;
    }

    [[nodiscard]] float get_time() const
    {
        return m_time;
    }

    [[nodiscard]] float get_length() const
    {
        return m_length;
    }

    void reset_timer()
    {
        m_time = 0;
    }
private:
    float m_length;
    float m_time;
    bool m_time_up;
};
