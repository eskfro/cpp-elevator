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
    Any call to this object must be protected with the peers_mutex_ in Node class.
    */
    public:
        Peers();

        void Step(int elev_id);
        void UpdateAllCosts(int elev_id);

        // Get
        elev::ordersync::OrderMatrix* Matrix(int elev_id);
        elev::elevator::ElevatorState* State(int elev_id);
        int NumElevs();
        uint64_t Version(int elev_id);

        // Set
        void SetMatrix(int elev_id, elev::ordersync::OrderMatrix matrix);
        void SetState(int elev_id, elev::elevator::ElevatorState state);
        void SetNumElevs(int num_elevs);
        void SetClearOrders(int elev_id, int floor, ButtonFlags b2c);
        void IncrementVersion(int elev_id);
        void SetVersion(int elev_id, uint64_t version);

        // Sync
        void SyncMatrix(int elev_id);
        void AcceptPotentialOrders(int elev_id);
        int ElevatorWithLowestCost();
        bool RequestedByAll(int elev_id, int floor, elev::common::BtnType btn);

    
    private:
        int num_elevs_{};
        uint64_t versions_[elev::config::N_ELEVS]{};
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> all_matrices_{};
        std::array<elev::elevator::ElevatorState, elev::config::N_ELEVS> all_states_{};
        std::array<int, elev::config::N_ELEVS> all_costs_{};
};

} // namespace elev::network