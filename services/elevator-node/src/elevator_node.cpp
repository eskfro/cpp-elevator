#include <mutex>
#include <thread>

// Libs
#include "common/types.hpp"
#include "elevator/elevator.hpp"
#include "network/udp_bcast.hpp"
#include "ordersync/ordersync.hpp"

// Service
#include <elevator-node/elevator_node.hpp>

using namespace std::chrono_literals;

namespace elev::node {


bool ElevatorNode::Running() {
    return running_.load();
}


ElevatorNode::ElevatorNode(int ID, std::string IP) :
    node_id_(ID),
    running_(true),
    elev_(ID, IP) { 
        Init();
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
    SyncPeers();


    // 
    if (elev_.StopSignal()) {
        Event(controller_._fsm_emergency_stop(&elev_));
        SyncRequestTableFromOrderMatrix();
    }
    if (elev_.HitNewFloor()) {
        Event(controller_._fsm_floor_arrival(&elev_));
        SyncRequestTableFromOrderMatrix();
    }
    if (controller_.RequestTableUpdated()) { 
        Event(controller_._fsm_table_update(&elev_));
        SyncRequestTableFromOrderMatrix();
    }
    if (controller_.Doortimer()->Expired()) {
        Event(controller_._fsm_door_timeout(&elev_));
        SyncRequestTableFromOrderMatrix();
    }

    UpdatePeerElevState();
};


void ElevatorNode::SyncPeers() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    peers_.Step(node_id_);
    
}


void ElevatorNode::UpdatePeerElevState() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    peers_.State(NodeID())->CopyFrom(elev_.State());
}


void ElevatorNode::Init() {
    elev_.Init();
}


void ElevatorNode::Stop() {
    running_.store(false);
}


elev::network::NetworkPacket ElevatorNode::TxPacketCopy() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    elev::network::NetworkPacket packet;

    peers_.IncrementVersion(NodeID());
    packet.Update(peers_.Matrix(NodeID()), peers_.State(NodeID()));
    packet.SetVersion(peers_.Version(NodeID()));
    
    return packet;

}


void ElevatorNode::Event(ButtonFlags b2c) {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    ButtonFlags zeros{};
    if (b2c != zeros) {
        peers_.SetClearOrders(NodeID(), elev_.State()->Floor(), b2c);
    }
}


int ElevatorNode::NodeID() {
   return node_id_;
}


// Sets the controller request-table according to the synced OrderMatrix orders
void ElevatorNode::SyncRequestTableFromOrderMatrix() {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    int e = NodeID();
    controller_.UpdateRequests(peers_.Matrix(e)->Table(e)->ToBoolTable());
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::UpdateOrderMatrixFromButtonSignals() {
    std::lock_guard<std::mutex> lock(peers_mutex_);

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
