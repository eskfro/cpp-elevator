#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>

// Service
#include <elevator-node/elevator_node.hpp>

using namespace std::chrono_literals;

namespace elev::node {


ElevatorNode::ElevatorNode() : running{true} {};


ElevatorNode::ElevatorNode(int _ID, std::string _IP) {
    running = true;
    elev.setID(_ID);
    elev.setIP(_IP);
}   


void ElevatorNode::loop() {
    int thisID = elev.getID();
    int prev_floor = elev.getFloorSensor();
    elev::control::RequestTable prev_requests{};

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    elev.initToFloor();

    while (running) {
        int cf = elev.getFloorSensor(); // raw floor sensor
        
        // Button press
        pollBtnSignals();
        syncRequests();
        setBtnLamps();

        // [ Event ] - NewFloor
        if (cf != prev_floor && cf != BETWEEN_FLOORS) {
            elev.setFloor(cf);
            prev_floor = elev.getFloor();
            peers.setClearOrders(thisID, elev.getFloor(), controller.fsm_floor_arrival(&elev));
        }

        // [ Event ] - TableUpdate
        bool req_changed = !controller.getRequests().is_equal(prev_requests);
        if (req_changed) {
            peers.setClearOrders(thisID, elev.getFloor(), controller.fsm_table_update(&elev));
            syncRequests();
            prev_requests = controller.getRequests();
        }

        // [ Event ] - DoorTimeout
        if (controller.getDoorTimer()->isExpired()) {
            controller.getDoorTimer()->stop();
            peers.setClearOrders(thisID, elev.getFloor(), controller.fsm_door_timeout(&elev));
        }

        std::this_thread::sleep_for(20ms);
    }
};

// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::syncRequests() {
    int thisID = this->elev.getID();
    elev::ordersync::OrderSlice localSlice = peers.getSliceFor(thisID);
    controller.updateRequests(localSlice);
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::pollBtnSignals() {
    using namespace elev::common;
    int thisID = this->elev.getID();

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (elev.getBtnSignal(f, (BtnType)b)) {
    
                printBtnPress(thisID, f, (BtnType)b);

                // TODO: set to requested when distr logic is inplace
                peers.registerBtnPress(thisID, f, (BtnType)b, OrderStatus::CONFIRMED);
                

            }
        }
    }
}


void ElevatorNode::setBtnLamps() {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (controller.getRequests().getValueAt(f, (BtnType)b)) {
                elev.setBtnLamp(f, (BtnType)b, true);
            } else {
                elev.setBtnLamp(f, (BtnType)b, false);
            }
        }
    }
}


} // end namespace
