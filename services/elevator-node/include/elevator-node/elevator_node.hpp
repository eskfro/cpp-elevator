#pragma once

#include "network/udp_bcast.hpp"
#include <stdio.h>
#include <string>
#include <array>
#include <mutex>
#include <atomic>

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
        void Stop();
        void Init();
        void Event(ButtonFlags b2c);

        // Getters
        bool Running();
        int NodeID();
        elev::network::NetworkPacket TxPacketCopy();

        // Sync modules
        void UpdatePeerElevState();
        void UpdateOrderMatrixFromButtonSignals();
        void SyncRequestTableFromOrderMatrix();
        void SetButtonLamps();


    private:
        int node_id_;
        std::atomic<bool> running_{true};
        elev::elevator::Elevator elev_;
        elev::control::Controller controller_;
        elev::network::Peers peers_;
        std::mutex peers_mutex_;
};

} //namespace elev::node
