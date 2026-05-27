#include <iostream>
#include <unistd.h>
#include <memory>

#include <elevator_node.hpp>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>
#include <elevator/elevator.hpp>
#include <control/controller.hpp>
#include <network/udp_bcast.hpp>

int main() {
    using namespace elev;

    std::cout << "=== MAIN THREAD ===" << std::endl;

    // TODO: parse this from cmd line
    int ID = 0;
    std::string IP = "localhost";
    
    hardware::initHardware();

    node::ElevatorNode node = node::ElevatorNode(ID, IP);
    
    node.loop();



};