#include "common/utils.hpp"
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

// Service
#include <elevator-node/elevator_node.hpp>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <control/controller.hpp>
#include <elevator/elevator.hpp>
#include <hardware/hardware.hpp>
#include <network/udp_bcast.hpp>

void TxThreadLoop(elev::node::ElevatorNode& node, elev::network::UdpBroadcaster& bcaster, const std::atomic<bool>& g_running);
void RxThreadLoop(elev::node::ElevatorNode& node, elev::network::UdpReciever& reciever, const std::atomic<bool>& g_running);

static constexpr uint16_t kPort = 3435;
std::atomic<bool> g_running{true};

void SigHandler(int) { g_running = false; }

int main(int argc, char* argv[]) {
    std::signal(SIGINT, SigHandler);
    std::signal(SIGTERM, SigHandler);
    elev::common::Print("=== ELEVATOR NODE ===");

    int id, sim = 0;
    if (argc < 3) elev::common::Abort("[MAIN] Usage: elevator-node <id> <sim>");
    try {id = std::stoi(argv[1]);} catch (const std::exception& e) {elev::common::PrintError(e.what()); return 1;};
    try {sim = std::stoi(argv[2]);} catch (const std::exception& e) {elev::common::PrintError(e.what()); return 1;};

    std::string ip = "localhost";
    elev::hardware::init_hardware(id, sim);

    auto node = elev::node::ElevatorNode(id, ip);
    node.Init();
    node.Step();

    // Network threads
    auto bcaster = elev::network::UdpBroadcaster(kPort, "255.255.255.255");
    auto reciever = elev::network::UdpReciever(kPort);
    std::thread rx_thread(RxThreadLoop, std::ref(node), std::ref(reciever), std::cref(g_running));
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
    node.Stop();
    elev::common::Print("=== SHUTTING DOWN ===");
    if (rx_thread.joinable()) rx_thread.join();
    if (tx_thread.joinable()) tx_thread.join();
    reciever.Close();
    elev::hardware::shutdown_hardware();

    return 0;
};

void RxThreadLoop(elev::node::ElevatorNode& node, elev::network::UdpReciever& reciever, const std::atomic<bool>& g_running) {
    elev::network::NetworkPacket packet;
    while (g_running && node.Running()) {
        // Blocks until network frame arrives
        if (reciever.RecievePacket(&packet)) {
            if (packet.Id() == node.Id()) continue;
            node.RxPacketProcessing(packet);
            std::cout << "[RX] Processed package " << packet.Id() << std::endl;
        }
    }
}

void TxThreadLoop(elev::node::ElevatorNode& node, elev::network::UdpBroadcaster& bcaster, const std::atomic<bool>& g_running) {
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