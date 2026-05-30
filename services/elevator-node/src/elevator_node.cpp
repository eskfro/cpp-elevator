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


void ElevatorNode::eventLoop() {
    int thisID = elev.getID();
    int prev_floor = elev.getFloorSensor();
    bool prev_stop = elev.getStopSignal();
    elev::control::RequestTable prev_requests{};

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    elev.initToFloor();

    while (running) {
        
        // Button press
        checkObs();
        checkBtnSignals();
        syncRequests();
        setBtnLamps();
        
        // [ Event ] - Emergency Stop
        bool stop = elev.getStopSignal(); // Current Stop Signal
        if (stop != prev_stop) {
            elev.setStop(stop);
            event(controller.fsm_emergency_stop(&elev));
            prev_stop = stop;
        }
        
        // [ Event ] - NewFloor
        int cf = elev.getFloorSensor(); // Current Floor
        if (cf != prev_floor && cf != BETWEEN_FLOORS) {
            elev.setFloor(cf);
            event(controller.fsm_floor_arrival(&elev));
            prev_floor = elev.getFloor();
        }

        // [ Event ] - TableUpdate
        bool req_changed = !controller.getRequests().is_equal(prev_requests);
        if (req_changed) {
            event(controller.fsm_table_update(&elev));
            prev_requests = controller.getRequests();
        }

        // [ Event ] - DoorTimeout
        if (controller.getDoorTimer()->isExpired()) {
            controller.getDoorTimer()->stop();
            event(controller.fsm_door_timeout(&elev));
        }

        std::this_thread::sleep_for(25ms);
    }
};


void ElevatorNode::checkObs() {
    bool obstruction = elev.getObsSignal();
    elev.setObs(obstruction);
}


void ElevatorNode::event(ButtonFlags b2c) {

    ButtonFlags zeros{};

    if (b2c != zeros) {
        peers.setClearOrders(elev.getID(), elev.getFloor(), b2c);
        syncRequests();
    }

}


// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::syncRequests() {
    int thisID = this->elev.getID();
    elev::ordersync::OrderSlice localSlice = peers.getSliceFor(thisID);
    controller.updateRequests(localSlice);
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::checkBtnSignals() {
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
