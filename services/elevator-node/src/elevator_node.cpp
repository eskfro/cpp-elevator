#include <memory>
#include <thread>
#include <chrono>

// Service
#include <elevator-node/elevator_node.hpp>

using namespace std::chrono_literals;

namespace elev::node {

void ElevatorNode::loop() {

    int thisID = elev.getID();
    int prev_floor = elev.getFloorSensor();

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    while (running) {

        // IN:
        // - Button press
        
        // OUT
        // - (Open / Close) door
        // - Set motordir

        // EVENTS:
        // - New floor
        // - Door timer
        // - Button Press
        // - New networkpacket


        int floor = elev.getFloorSensor();
        if (floor != prev_floor) {
            controller.fsm_floor_arrival(&elev, floor);
            prev_floor = floor;
        }

        pollBtnSignals(peers.getOrdersAt(thisID));

          



        

        



        
        std::this_thread::sleep_for(20ms);
    }


};

// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::setRequestTable(elev::control::RequestTable* requests, elev::ordersync::OrderMatrix* orders) {
    // TODO: mutex
    using namespace elev::common;
    int thisID = this->elev.getID();
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (orders->getStatusAt(thisID, f, (BtnType)b) == OrderStatus::CONF) {
                requests->setValueAt(f, (BtnType)b, 1);
            }
        }
    }
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::pollBtnSignals(elev::ordersync::OrderMatrix* orders) {
    using namespace elev::common;
    // TODO: mutex
    int thisID = this->elev.getID();

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (elev.getBtnSignal(f, (BtnType)b)) {
                orders->setStatusAt(thisID, f, (BtnType)b, OrderStatus::CONF); 
                // TODO: Set to REQ when distr logic is in place
            }
        }
    }
}


}
