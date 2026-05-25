#include <elevator-node/elevator_node.hpp>

using namespace elev;
namespace elev::node {

void ElevatorNode::loop() {

    ElevatorNode node;

    node.elev = elevator::Elevator(0, "localhost");
    node.controller = control::Controller();
    node.peers = network::Peers();

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    while (true) {

        // EVENTS:

        // New floor

        // Button Press

        // New networkpacket

        
        

    }


}



}

