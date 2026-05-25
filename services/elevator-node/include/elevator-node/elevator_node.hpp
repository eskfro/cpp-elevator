#pragma once

#include <control/controller.hpp>
#include <ordersync/ordersync.hpp>
#include <network/udp_bcast.hpp>

namespace elev::node {

// The node connecting things
class ElevatorNode {
    private:
        // hardware abstraction
        elev::elevator::Elevator elev;

        // hardware controller
        elev::control::Controller controller;

        // overview of all elevator-nodes :)
        elev::network::Peers peers;
    
    public:
        ElevatorNode();
        ~ElevatorNode();
        void loop(); // lol
        
};





}
