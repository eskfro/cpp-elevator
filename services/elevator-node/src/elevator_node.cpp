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

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        StepPeers();
    }

    SetButtonLamps();

    if (elev_.StopSignal()) {
        Event(controller_.FsmEmergencyStop(&elev_));
    }
    if (elev_.HitNewFloor()) {
        Event(controller_.FsmFloorArrival(&elev_));
    }
    if (controller_.RequestTableUpdated()) { 
        Event(controller_.FsmTableUpdate(&elev_));
    }
    if (controller_.DoorTimer()->Expired()) {
        Event(controller_.FsmDoorTimeout(&elev_));
    }

};

void ElevatorNode::StepPeers() {
    UpdateOrderMatrixFromButtonSignals();
    peers_.State(node_id_)->CopyFrom(elev_.State());
    peers_.Step(node_id_);
}

void ElevatorNode::RxPacketProcessing(network::NetworkPacket packet) {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    using namespace elev::ordersync;
    
    peers_.State(packet.ID())->OnUpdate(*packet.State());
    peers_.Orders()->Join(*packet.Orders());
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
    packet.Init(peers_.Orders(), peers_.State(ID()));

    return packet; 
}


void ElevatorNode::Event(ButtonFlags b2c) {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    // Set clear orders
    ButtonFlags zeros{};
    if (b2c != zeros) {
        peers_.SetClearOrders(ID(), elev_.State()->Floor(), b2c);
    }

    const int n = node_id_;
    controller_.SetRequests(peers_.Orders()->Table(n)->ToBoolTable());
    elev_.State()->SetRequests(controller_.Requests().Table());
    peers_.State(n)->CopyFrom(elev_.State());
    peers_.State(n)->IncrementVersion();
}

int ElevatorNode::ID() {
   return node_id_;
}

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::UpdateOrderMatrixFromButtonSignals() {
    using namespace elev::common;
    const int n = node_id_;

    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if (elev_.Buttons()->Button(f, (BtnType)b)->Pressed()) {
                PrintBtnPress(n, f, (BtnType)b);
                peers_.Orders()->Table(n)->Order(f, b)->OnRequest();
            }
        }
    }
    controller_.SetRequests(peers_.Orders()->Table(n)->ToBoolTable());
}

void ElevatorNode::SetButtonLamps() {
    using namespace elev::common;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if (controller_.Requests().Value(f, b)) {
                elev_.SetButtonLamp(f, (BtnType)b, true);
            } else {
                elev_.SetButtonLamp(f, (BtnType)b, false);
            }
        }
    }
}

} // namespace elev::node
