#include <mutex>
#include <thread>

// Libs
#include "common/config.hpp"
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
    SetButtonLamps();
    SyncPeers();

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

void ElevatorNode::RxPacketProcessing(network::NetworkPacket packet) {
    /*
    Do things when the node rcv's an udp packet
    */
    using namespace elev::ordersync;
    OrderTable packet_table = *packet.Matrix()->Table(packet.ID());

    // all_states_ update
    peers_.State(packet.ID())->OnUpdate(*packet.State());

    // all_matrices_ updates
    // update this nodes matrix
    peers_.Matrix(node_id_)->Table(packet.ID())->Join(packet_table);

    // update what I think packet.ID() table looks like
    for (int e = 0; e < N_ELEVS; e++) {
        OrderTable packet_table = *packet.Matrix()->Table(e);
        peers_.Matrix(packet.ID())->Table(e)->Join(packet_table);
    }
    
}


void ElevatorNode::SyncPeers() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
    peers_.Step(node_id_);
    
}


void ElevatorNode::UpdatePeerElevState() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    
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

    packet.Update(peers_.Matrix(NodeID()), peers_.State(NodeID()));

    return packet; 
}


void ElevatorNode::Event(ButtonFlags b2c) {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    // Set clear orders
    ButtonFlags zeros{};
    if (b2c != zeros) {
        peers_.SetClearOrders(NodeID(), elev_.State()->Floor(), b2c);
    }

    int n = node_id_;
    controller_.UpdateRequests(peers_.Matrix(n)->Table(n)->ToBoolTable());
    peers_.State(n)->CopyFrom(elev_.State());
    peers_.State(n)->IncrementVersion();

}


int ElevatorNode::NodeID() {
   return node_id_;
}


// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::UpdateOrderMatrixFromButtonSignals() {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    using namespace elev::common;
    const int e = node_id_;

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (elev_.Buttons()->Button(f, (BtnType)b)->Pressed()) {
                PrintBtnPress(e, f, (BtnType)b);
                peers_.Matrix(e)->Table(e)->Order(f, b).OnRequest();
            }
        }
    }

    controller_.UpdateRequests(peers_.Matrix(e)->Table(e)->ToBoolTable());
}


void ElevatorNode::SetButtonLamps() {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (controller_.Requests().Value(f, b)) {
                elev_.SetButtonLamp(f, (BtnType)b, true);
            } else {
                elev_.SetButtonLamp(f, (BtnType)b, false);
            }
        }
    }
}


} // namespace elev::node
