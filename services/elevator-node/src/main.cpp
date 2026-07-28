#include <functional>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <csignal>

#include <elevator-node/elevator_node.hpp>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>
#include <elevator/elevator.hpp>
#include <control/controller.hpp>
#include <network/udp_bcast.hpp>

void TxThreadLoop(elev::node::ElevatorNode& node, elev::network::UdpBroadcaster& bcaster, const std::atomic<bool>& g_running);

std::atomic<bool> g_running{true};
void SigHandler(int) {
    g_running = false;
}

int main() {
    std::signal(SIGINT, SigHandler);
    std::signal(SIGTERM, SigHandler);
    elev::common::Print("=== ELEVATOR NODE ===");
    
    // Init
    int ID = 0;
    std::string IP = "localhost";
    elev::hardware::init_hardware();
    elev::node::ElevatorNode node = elev::node::ElevatorNode(ID, IP);
    elev::network::UdpBroadcaster bcaster = elev::network::UdpBroadcaster(3435, "255.255.255.255");

    // Tx thread
    std::thread tx_thread(TxThreadLoop, std::ref(node), std::ref(bcaster), std::cref(g_running));

    // Control loop
    constexpr auto kSampleTime = std::chrono::milliseconds(40);
    auto next_tick = std::chrono::steady_clock::now();
    while (g_running && node.Running()) {
        node.Step();
        next_tick += kSampleTime;
        std::this_thread::sleep_until(next_tick);
    }

    // Shutdown
    elev::common::Print("=== SHUTTING DOWN ===");
    node.Stop();
    if (tx_thread.joinable()) tx_thread.join();
    
    return 0;
};


void TxThreadLoop(
    elev::node::ElevatorNode& node,
    elev::network::UdpBroadcaster& bcaster,
    const std::atomic<bool>& g_running
) {
    constexpr auto kTxPeriod = std::chrono::milliseconds(50);
    auto next_tx = std::chrono::steady_clock::now();

    while (g_running && node.Running()) {
        elev::network::NetworkPacket packet = node.TxPacketCopy();

        if (!bcaster.SendPacket(&packet)) elev::common::PrintError("[TX thread] Failed to send packet");
        
        next_tx += kTxPeriod;
        const auto now = std::chrono::steady_clock::now();
        if (next_tx < now) {
            next_tx = now + kTxPeriod;
            PrintError("[TX thread] Code took longer to run than kTxPeriod");
        }    
        std::this_thread::sleep_until(next_tx);
    }
}