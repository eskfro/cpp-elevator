#pragma once

#include <control/controller.hpp>
#include <ordersync/ordersync.hpp>


using PeerList = std::array<elev::node::Peer, elev::config::N_ELEVS>;

namespace elev::node {

// The node connecting things
class ElevatorNode {
    private:
        // hardware abstraction
        elev::elevator::Elevator elev;

        // hardware controller
        elev::control::Controller controller;

        // overview of all elevator-nodes :)
        Peers peers;
    
    public:
        void start(); // lol
        
};


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
        elev::elevator::Elevator elev;
        elev::ordersync::OrderMatrix orders;
    
    public:
        Peer();
};


}
