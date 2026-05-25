#pragma once

#include <array>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

using PeerList = std::array<elev::network::Peer, elev::config::N_ELEVS>;

namespace elev::network {

class Peers {
    private:
        int numElevs;
        PeerList peers;

    public:
        Peers();
        int getNumElevs();
        void setNumElevs(int n);
};


class Peer {
    private:
        elev::common::ElevatorState state;
        elev::ordersync::OrderMatrix orders;
    
    public:
        Peer();
};

}