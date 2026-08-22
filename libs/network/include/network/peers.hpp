#pragma once

#include "elevator/elevator_state.hpp"
#include "network/order_timers.hpp"
#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <cstdint>
#include <ordersync/ordersync.hpp>
#include <elevator/elevator.hpp>

namespace {

static constexpr int kPenaltyDoorOpen = 2;
static constexpr int kPenaltyFloorDiff = 3;
static constexpr int kPenaltyPerOrder = 5;
static constexpr int kPenaltyWrongDir = 10;
static constexpr int kPenaltyObstruction = 100;

} //namespace

namespace elev::network {

/*
This is the world object.
*/
class Peers {
    public:
        Peers() = default;
        
        void Step(int node_id);
        void UpdateNumElevs();
        bool ObservedByAll(int floor, int btn);
        int ElevatorWithLowestCost(int floor, int btn);

        // Get
        int NumElevs();
        elev::ordersync::OrderTable* Orders();
        elev::elevator::ElevatorState* State(int elev_id);
        std::array<std::array<elev::ordersync::Order, kFloors>, kElevs>* CabButtonOrders();
        elev::ordersync::Order* CabButtonOrder(int elev_id, int floor);
        
        // Orders
        void CheckOrderTimers(); // TODO
        void ObserveOrders(int node_id);
        void ConfirmHallOrders(int node_id);
        void ResetHallOrders(int node_id);
        void ClearOrders(int node_id, int floor, ButtonFlags b2c);

    private:
        int num_elevs_{};
        ordersync::OrderTable orders_{};
        OrderTimers order_timers_{};
        std::array<elev::elevator::ElevatorState, elev::config::kElevs> all_states_{};
        std::array<std::array<elev::ordersync::Order, kFloors>, kElevs> cab_button_orders_{};
};

} // namespace elev::network