#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>

#include <elevator-node/elevator_node.hpp>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>
#include <elevator/elevator.hpp>
#include <control/controller.hpp>
#include <network/udp_bcast.hpp>

int main() {

    std::cout << "=== MAIN THREAD ===" << std::endl;

    // TODO:
    // parse this from cmd line
    int ID = 0;
    std::string IP = "localhost";
    
    elev::hardware::init_hardware();

    elev::node::ElevatorNode node = elev::node::ElevatorNode(ID, IP);

    // TODO: note start time with clock

    constexpr auto kSampleTime = std::chrono::milliseconds(40);
    auto next_tick = std::chrono::steady_clock::now();

    while (node.Running()) {
        
        node.Step();

        next_tick += kSampleTime;
        std::this_thread::sleep_until(next_tick);
    }

};