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
    void Stop();
    void SyncPeers();
    void Event(ButtonFlags b2c);

    // Get
    int Id() const;
    bool Running() const;

    // Sync modules
    void SetButtonLamps();
    void SyncButtonSignals();
    void RegisterButtonSignals();

    // Network
    elev::network::NetworkPacket TxPacketCopy();
    void RxPacketProcessing(network::NetworkPacket packet);

private:
    int node_id_;
    ServiceState service_state_{};
    std::atomic<bool> running_{true};

    BoolTable button_signals_{};
    BoolTable button_lamps_{};

    std::mutex peers_mutex_;
    elev::network::Peers peers_;
    elev::elevator::Elevator elev_;
    elev::control::Controller controller_;
};

}  // namespace elev::node
