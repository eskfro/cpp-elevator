#include "network/order_timers.hpp"

#include "common/timer.hpp"

namespace elev::network {

elev::common::Timer* OrderTimers::Timer(int floor, int btn) {
    return &timers_[floor][btn];
}

}  // namespace elev::network