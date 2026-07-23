#include "ordersync/ordersync.hpp"
#include <thread>

// Service
#include <elevator-node/elevator_node.hpp>

using namespace std::chrono_literals;

namespace elev::node {


ElevatorNode::ElevatorNode() : running_{true} {};


ElevatorNode::ElevatorNode(int _ID, std::string _IP) {
    running_ = true;
    elev_.SetID(_ID);
    elev_.SetIP(_IP);
}   


void ElevatorNode::EventLoop() {
    int thisID = elev_.ID();
    int prev_floor = elev_.GetFloorSensor();
    bool prev_stop = false;
    elev::control::RequestTable prev_requests{};

    // begin network bcast thread;
    // begin network reciever thread that writes to peers;
    // begin ordersync thread

    elev_.InitToFloor();

    while (running_) {

        // TODO:
        // Queue of OrderMatrix's sent over the network from the other nodes
        //for (matrix : peers_.matrixQueue_) {
        //    peers_.MergeIncomingMatrix(int matr, elev::ordersync::OrderMatrix matrix)
        //}
        
        // Button press
        CheckObs();
        CheckBtnSignals();
        SyncRequests();
        SetBtnLamps();
        
        // [ Event ] - Emergency Stop
        if (elev_.GetStopSignal()) {
            Event(controller_._fsm_emergency_stop(&elev_));
        }
        // [ Event ] - NewFloor
        int cf = elev_.GetFloorSensor();
        if (cf != prev_floor && cf != BETWEEN_FLOORS) {
            elev_.SetFloor(cf);
            Event(controller_._fsm_floor_arrival(&elev_));
            prev_floor = elev_.Floor();
        }

        // [ Event ] - TableUpdate
        bool req_changed = controller_.IsRequestsChanged(prev_requests);
        if (req_changed) {
            Event(controller_._fsm_table_update(&elev_));
            prev_requests = controller_.Requests();
        }

        // [ Event ] - DoorTimeout
        if (controller_.Doortimer()->Expired()) {
            controller_.Doortimer()->Stop();
            Event(controller_._fsm_door_timeout(&elev_));
        }

        std::this_thread::sleep_for(25ms);
    }
};


void ElevatorNode::CheckObs() {
    elev_.SetObs(elev_.GetObsSignal()); // set obs from sensor
}


void ElevatorNode::Event(ButtonFlags b2c) {

    ButtonFlags zeros{};

    if (b2c != zeros) {
        peers_.SetClearOrders(elev_.ID(), elev_.Floor(), b2c);
        SyncRequests();
    }

}


// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::SyncRequests() {
    int e = elev_.ID();
    controller_.UpdateRequests(peers_.Matrix(e)->Table(e)->ToBoolTable());
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::CheckBtnSignals() {
    using namespace elev::common;
    int e = this->elev_.ID();

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (elev_.GetBtnSignal(f, (BtnType)b)) {
    
                PrintBtnPress(e, f, (BtnType)b);

                // TODO: set to requested when distr logic is inplace
                peers_.Matrix(e)->Table(e)->SetStatus(f, (BtnType)b, OrderStatus::CONFIRMED);
                

            }
        }
    }
}


void ElevatorNode::SetBtnLamps() {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (controller_.Requests().Value(f, (BtnType)b)) {
                elev_.SetBtnLamp(f, (BtnType)b, true);
            } else {
                elev_.SetBtnLamp(f, (BtnType)b, false);
            }
        }
    }
}


} // end namespace
