#pragma once

#include "elevator/elevator_state.hpp"
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

        // Get
        int NumElevs();
        elev::ordersync::OrderTable* Orders();
        elev::elevator::ElevatorState* State(int elev_id);
        elev::ordersync::Order* CabButtonOrder(int elev_id, int floor);
        std::array<std::array<elev::ordersync::Order, kFloors>, kElevs>* CabButtonOrders();
        
        // Orders
        void ObserveOrders(int node_id);
        void ConfirmOrders(int node_id);
        void ResetOrders(int node_id);
        void ClearOrders(int node_id, int floor, ButtonFlags b2c);

        void UpdateNumElevs();
        
        bool ObservedByAll(int floor, int btn);

        int ElevatorWithLowestCost(int floor, int btn);

    private:
        int num_elevs_{};
        ordersync::OrderTable orders_{};
        std::array<elev::elevator::ElevatorState, elev::config::kElevs> all_states_{};
        std::array<std::array<elev::ordersync::Order, kFloors>, kElevs> cab_button_orders_{};
};

} // namespace elev::network