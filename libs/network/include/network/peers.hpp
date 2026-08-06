#pragma once

#include "elevator/elevator_state.hpp"
#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <cstdint>
#include <ordersync/ordersync.hpp>
#include <elevator/elevator.hpp>

namespace elev::network {

struct CostPenalties {
    int floor_diff = 3;
    int num_orders = 3;
    int wrong_dir = 10;
    int obstruction = 100;
};

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
        void AcceptPotentialOrders(int elev_id);
        int ElevatorWithLowestCost();
        bool RequestedByAll(int elev_id, int floor, elev::common::BtnType btn);

    
    private:
        int num_elevs_{};
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> all_matrices_{};
        std::array<elev::elevator::ElevatorState, elev::config::N_ELEVS> all_states_{};
        std::array<int, elev::config::N_ELEVS> all_costs_{};
};

} // namespace elev::network