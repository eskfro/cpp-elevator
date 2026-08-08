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
static constexpr int kPenaltyPerOrder = 3;
static constexpr int kPenaltyWrongDir = 10;
static constexpr int kPenaltyObstruction = 100;

} //namespace



namespace elev::network {

class Peers {
    /*
    This is the world object.
    */
    public:
        Peers();

        void Step(int node_id);

        // Get
        elev::ordersync::OrderMatrix* Orders();
        elev::elevator::ElevatorState* State(int elev_id);
        int NumElevs();

        // Set
        void SetNumElevs(int num_elevs);
        void SetClearOrders(int node_id, int floor, ButtonFlags b2c);

        void ConfirmOrders(int node_id);
        void ResetOrders(int node_id);
        int ElevatorWithLowestCost(int floor, int btn);
        bool RequestedByAll(int floor, int btn);

        void UpdateNumElevs();

    
    private:
        int num_elevs_{};
        ordersync::OrderMatrix orders_{};
        std::array<elev::elevator::ElevatorState, elev::config::kElevs> all_states_{};
};

} // namespace elev::network