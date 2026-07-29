#pragma once

#include "elevator/elevator_state.hpp"
#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <ordersync/ordersync.hpp>
#include <elevator/elevator.hpp>

namespace elev::network {

class Peers {
    /*
    This is the world object.
    Any call to this object must be protected with the peers_mutex_ in Node class.
    */
    public:
        Peers();

        // Getters
        elev::ordersync::OrderMatrix* Matrix(int elev_id);
        elev::elevator::ElevatorState* State(int elev_id);
        int NumElevs();
        uint64_t Version(int elev_id);

        void SetMatrix(int elevID, elev::ordersync::OrderMatrix matrix);
        void SetNumElevs(int num_elevs);
        void MergeIncomingMatrix(int elevID, elev::ordersync::OrderMatrix matrix);
        void SetClearOrders(int elevID, int floor, ButtonFlags b2c);
        void IncrementVersion(int elev_id);
    
    private:
        int num_elevs_{};
        uint64_t versions_[elev::config::N_ELEVS]{};
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> all_matrices_{};
        std::array<elev::elevator::ElevatorState, elev::config::N_ELEVS> all_states_{};

};

} // namespace elev::network