#pragma once

#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::network {

class Peers {
    private:
        int numElevs;
        std::array<elev::ordersync::OrderMatrix, elev::config::N_ELEVS> allOrders;
        std::array<elev::common::ElevatorState, elev::config::N_ELEVS> allElevs;

    public:
        Peers();
        int getNumElevs();

        elev::ordersync::OrderMatrix* getOrdersAt(int elevID);

        void setNumElevs(int n);
        void setStatusAt(int elevID, int floor, BtnType btn);
};

}