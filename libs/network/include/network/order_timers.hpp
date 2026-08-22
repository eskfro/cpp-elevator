#pragma once

#include "common/config.hpp"
#include "common/timer.hpp"
#include <array>

namespace elev::network {

using TimerTable = std::array<std::array<elev::common::Timer, elev::config::kButtons>, elev::config::kFloors>;
    
class OrderTimers {
    public:
        OrderTimers() = default;

        TimerTable* Timers() { return &timers_; }
        elev::common::Timer* Timer(int floor, int btn);


    private:
        TimerTable timers_{};

};

} // namespace elev::network