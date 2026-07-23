#pragma once

#include <stdio.h>
#include <string>
#include <array>

// Libs
#include <control/controller.hpp>
#include <ordersync/ordersync.hpp>
#include <network/peers.hpp>

namespace elev::node {

// The node connecting things
class ElevatorNode {

    public:
        ElevatorNode();
        ElevatorNode(int _ID, std::string _IP);

        void EventLoop();
        void Event(ButtonFlags b2c);

        // Polling shi
        void CheckBtnSignals();
        void CheckStopSignal();
        void CheckObs();

        void SyncRequests();
        void SetBtnLamps();


    private:
        bool running_ = true;
        elev::elevator::Elevator elev_;
        elev::control::Controller controller_;
        elev::network::Peers peers_;
};

} //namespace elev::node
