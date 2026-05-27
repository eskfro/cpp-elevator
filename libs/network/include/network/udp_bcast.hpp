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
        
        elev::ordersync::OrderSlice getSliceFor(int elevID);
        
        void registerBtnPress(int elevID, int floor, BtnType btn, OrderStatus status);
        
        int getNumElevs();
        void setNumElevs(int n);

};

}