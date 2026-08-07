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

        void Step(int elev_id);
        void UpdateAllCosts(int elev_id);

        // Get
        elev::ordersync::OrderMatrix* Matrix(int elev_id);
        elev::elevator::ElevatorState* State(int elev_id);
        int NumElevs();

        // Set
        void SetMatrix(int elev_id, elev::ordersync::OrderMatrix matrix);
        void SetState(int elev_id, elev::elevator::ElevatorState state);
        void SetNumElevs(int num_elevs);
        void SetClearOrders(int elev_id, int floor, ButtonFlags b2c);
        void IncrementVersion(int elev_id);

        // Sync
        void ConfirmOrders(int node_id);
        int ElevatorWithLowestCost(int floor, int btn);
        bool RequestedByAll(int node_id, int floor, int btn);

    
    private:
        int num_elevs_{};
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> all_matrices_{};
        std::array<elev::elevator::ElevatorState, elev::config::N_ELEVS> all_states_{};
};

} // namespace elev::network