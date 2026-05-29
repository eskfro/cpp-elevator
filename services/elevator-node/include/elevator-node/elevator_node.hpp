#pragma once

// Libs
#include <control/controller.hpp>
#include <ordersync/ordersync.hpp>
#include <network/udp_bcast.hpp>

namespace elev::node {

// The node connecting things
class ElevatorNode {
    private:
        bool running = true;
        
        // hardware abstraction
        elev::elevator::Elevator elev;

        // hardware controller
        elev::control::Controller controller;

        // overview of all elevator-nodes :)
        elev::network::Peers peers;
    
    public:
        ElevatorNode();
        ElevatorNode(int _ID, std::string _IP);

        void eventLoop();
        void event(ButtonFlags b2c);

        // Polling shi
        void pollBtnSignals();
        void pollStopSignal();
        int pollFloorSensor();
        void pollObs();

        void syncRequests();
        void setBtnLamps();
        
};





}
