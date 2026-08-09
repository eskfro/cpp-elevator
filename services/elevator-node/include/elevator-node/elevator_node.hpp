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

class ElevatorNode {
    public:
        ElevatorNode() = delete;
        ElevatorNode(int id, std::string ip);

        void Step();
        void StepPeers();
        void Stop();
        void Init();
        void Event(ButtonFlags b2c);

        // Get
        bool Running();
        int Id();
        
        // Sync modules
        void UpdateOrderMatrixFromButtonSignals();
        void SetButtonLamps();

        // Network
        elev::network::NetworkPacket TxPacketCopy();
        void RxPacketProcessing(network::NetworkPacket packet);

    private:
        int node_id_;
        std::atomic<bool> running_{true};
        elev::elevator::Elevator elev_;
        elev::control::Controller controller_;
        elev::network::Peers peers_;
        std::mutex peers_mutex_;
};

} //namespace elev::node
