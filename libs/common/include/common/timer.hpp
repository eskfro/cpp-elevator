#pragma once

#include <chrono>

namespace elev::common {

class DoorTimer {
    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::milliseconds;

        TimePoint m_endTime;
        bool m_active{false};

    public:
        DoorTimer() = default;

        void start(int durationMs) {
            m_endTime = Clock::now() + Duration(durationMs);
            m_active = true;
        }

        // Stops the timer explicitly
        void stop() {
            m_active = false;
        }

        // Checks if the timer has reached its end time
        bool isExpired() const {
            if (!m_active) return false;
            return Clock::now() >= m_endTime;
        }

        bool isActive() const {
            return m_active;
        }
};

}