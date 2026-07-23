#pragma once

#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::network {

class Peers {
    private:
        int num_elevs_;
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> all_matrices_;
        std::array<elev::common::ElevatorState, elev::config::N_ELEVS> all_elev_states_;

    public:
        Peers();
                
        elev::ordersync::OrderMatrix* Matrix(int elevID);
        int NumElevs();

        void SetMatrix(int elevID, elev::ordersync::OrderMatrix matrix);
        void SetNumElevs(int n);
        void MergeIncomingMatrix(int elevID, elev::ordersync::OrderMatrix matrix);
        void SetClearOrders(int elevID, int floor, ButtonFlags b2c);

};

} // namespace elev::network