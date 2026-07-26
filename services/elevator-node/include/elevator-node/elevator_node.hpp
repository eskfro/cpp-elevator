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
        ElevatorNode() = delete;
        ElevatorNode(int ID, std::string IP);

        void Step();
        void InitElevator();
        void Event(ButtonFlags b2c);

        // Getters
        bool Running();

        // Polling shi
        void UpdateOrderMatrixFromButtonSignals();
        void CheckStopSignal();

        void SyncRequestTableFromOrderMatrix();
        
        void SetButtonLamps();


    private:
        bool running_;
        elev::elevator::Elevator elev_;
        elev::control::Controller controller_;
        elev::network::Peers peers_;
};

} //namespace elev::node
