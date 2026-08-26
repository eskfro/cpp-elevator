#pragma once

#include <array>

#include "common/timer.hpp"
#include "elevator/elevator_state.hpp"
#include "network/order_timers.hpp"

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <cstdint>
#include <elevator/elevator.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::network {

/*
This is the world object
*/
class Peers {
public:
    Peers() = default;

    void Step();
    void Init(int node_id);

    bool ObservedByAll(int floor, int btn);
    int ElevatorWithLowestCost(int floor, int btn);
    
    void UpdateNumElevs();
    void UpdateWatchdogTimer(int elev_id);
    void MonitorWatchdogTimers();
    void ControlHallOrderTimers();
    void MonitorHallOrderTimers();
    void MonitorFault();

    // Get
    int NumElevs();
    elev::ordersync::OrderTable* Orders();
    elev::elevator::ElevatorState* State(int elev_id);
    elev::ordersync::CabOrderTable* CabButtonOrders();
    elev::ordersync::Order* CabButtonOrder(int elev_id, int floor);

    // Orders
    void ObserveOrders();
    void ConfirmHallOrders();
    void ResetHallOrders();
    void ClearOrders(int floor, ButtonFlags b2c);
    void ReassignHallOrder(int floor, int btn);
    void ReassignHallOrders(int elev_id);

private:
    int node_id_{-1};
    int num_elevs_{};
    elev::ordersync::OrderTable orders_{};
    OrderTimers order_timers_{};
    std::array<elev::common::Timer, kElevs> watchdog_timers_{};
    std::array<elev::elevator::ElevatorState, elev::config::kElevs> all_states_{};
    elev::ordersync::CabOrderTable cab_button_orders_{};
};

}  // namespace elev::network