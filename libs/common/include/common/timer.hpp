#pragma once

#include <chrono>

namespace elev::common {

class Timer {
    public:
        Timer() = default;

        void Start(int duration_ms) {
            end_time_ = Clock::now() + Duration(duration_ms);
            active_ = true;
        }

        // Stops the timer explicitly
        void Stop() {
            active_ = false;
        }

        // Checks if the timer has reached its end time
        bool Expired() const {
            if (!active_) return false;
            return Clock::now() >= end_time_;
        }

        bool Active() const {
            return active_;
        }
    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::milliseconds;

        TimePoint end_time_;
        bool active_{false};
};

} // namespace elev::common