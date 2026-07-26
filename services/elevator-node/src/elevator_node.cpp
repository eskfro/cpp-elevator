#include <thread>

// Libs
#include "common/types.hpp"
#include "elevator/elevator.hpp"
#include "ordersync/ordersync.hpp"

// Service
#include <elevator-node/elevator_node.hpp>

using namespace std::chrono_literals;

namespace elev::node {


bool ElevatorNode::Running() {
    return running_;
}


ElevatorNode::ElevatorNode(int ID, std::string IP) :
    running_(true),
    elev_(ID, IP) {
        InitElevator();
    }   


void ElevatorNode::Step() {
    // TODO:
    // Queue of OrderMatrix's sent over the network from the other nodes
    //for (matrix : peers_.matrixQueue_) {
    //    peers_.MergeIncomingMatrix(int matr, elev::ordersync::OrderMatrix matrix)
    //}
    elev_.Step();

    UpdateOrderMatrixFromButtonSignals();
    SyncRequestTableFromOrderMatrix();
    SetButtonLamps();
    
    if (elev_.StopSignal()) {
        Event(controller_._fsm_emergency_stop(&elev_));
    }
    if (elev_.HitNewFloor()) {
        Event(controller_._fsm_floor_arrival(&elev_));
    }
    if (controller_.RequestTableUpdated()) {
        Event(controller_._fsm_table_update(&elev_));
    }
    if (controller_.Doortimer()->Expired()) {
        Event(controller_._fsm_door_timeout(&elev_));
    }

};


void ElevatorNode::InitElevator() {
    elev_.InitToFloor();
}



void ElevatorNode::Event(ButtonFlags b2c) {

    ButtonFlags zeros{};

    if (b2c != zeros) {
        peers_.SetClearOrders(elev_.State()->ID(), elev_.State()->Floor(), b2c);
        SyncRequestTableFromOrderMatrix();
    }

}


// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::SyncRequestTableFromOrderMatrix() {
    int e = elev_.State()->ID();
    controller_.UpdateRequests(peers_.Matrix(e)->Table(e)->ToBoolTable());
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::UpdateOrderMatrixFromButtonSignals() {
    using namespace elev::common;
    int e = elev_.State()->ID();

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (elev_.Buttons()->Button(f, (BtnType)b)->Pressed()) {
    
                PrintBtnPress(e, f, (BtnType)b);

                // TODO: set to requested when distr logic is inplace
                peers_.Matrix(e)->Table(e)->SetStatus(f, (BtnType)b, OrderStatus::CONFIRMED);
            }
        }
    }
}


void ElevatorNode::SetButtonLamps() {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (controller_.Requests().Value(f, (BtnType)b)) {
                elev_.SetButtonLamp(f, (BtnType)b, true);
            } else {
                elev_.SetButtonLamp(f, (BtnType)b, false);
            }
        }
    }
}


} // namespace elev::node
