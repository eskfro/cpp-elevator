#pragma once

#include <stdio.h>

#include <array>
#include <atomic>
#include <mutex>
#include <string>

#include "network/udp_bcast.hpp"

// Libs
#include <control/controller.hpp>
#include <network/peers.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::node {

enum class ServiceState : uint8_t { Startup, Running, Stopped };

class ElevatorNode {
public:
    ElevatorNode() = delete;
    ElevatorNode(int id, std::string ip);

    void Init();

    void Step();
    void StepPeers();
    void Stop();
    void Event(ButtonFlags b2c);

    // Get
    bool Running();
    int Id();

    // Sync modules
    void RegisterButtonSignals();
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

    ServiceState service_state_{};
};

}  // namespace elev::node
