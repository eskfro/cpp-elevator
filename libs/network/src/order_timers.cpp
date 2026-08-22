#include "common/timer.hpp"
#include "network/order_timers.hpp"

namespace elev::network {

elev::common::Timer* OrderTimers::Timer(int floor, int btn) {
    return &timers_[floor][btn];
}

} // namespace elev::network