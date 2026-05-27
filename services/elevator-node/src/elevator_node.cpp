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
    bool prev_doorState = false;
    elev::control::RequestTable prev_requests;
    ButtonFlags b2c{};

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    while (running) {
        
        // Button press
        pollBtnSignals();
    
        // Update controllers data
        setRequestTable();

        elev.setFloor(elev.getFloorSensor());

        // [ Event ] - NewFloor
        if (elev.getFloor() != prev_floor) {
            prev_floor = elev.getFloor();
            elev.setFloorIndicator();
            b2c = controller.fsm_floor_arrival(&elev);
        }

        // [ Event ] - TableUpdate
        if (controller.is_table_update(prev_requests)) {
            prev_requests.copy(controller.getRequestTable());
            b2c = controller.fsm_table_update(&elev);
        }

        // [ Event ] - DoorTimeout
        if (prev_doorState != elev.getDoorState() && elev.getDoorState() == false) {
            prev_doorState = elev.getDoorState();
            b2c = controller.fsm_door_timeout(&elev);
        }

        peers.setClearOrders(thisID, elev.getFloor(), b2c);

        std::this_thread::sleep_for(20ms);
    }


};

// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::setRequestTable() {
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
    
                // TODO: set to requested when distr logic is inplace
                peers.registerBtnPress(thisID, f, (BtnType)b, OrderStatus::CONFIRMED);

            }
        }
    }
}


}
