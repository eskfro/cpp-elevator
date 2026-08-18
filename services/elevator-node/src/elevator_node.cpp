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

ElevatorNode::ElevatorNode(int id, std::string ip) :
    node_id_(id),
    running_(true),
    elev_(id, ip) {}   

void ElevatorNode::Step() {
    
    elev_.Step();

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        StepPeers();
    }

    SetButtonLamps();

    if (elev_.StopSignal()) {
        Event(controller_.FsmEmergencyStop(&elev_));
        return;
    }
    if (elev_.HitNewFloor()) {
        Event(controller_.FsmFloorArrival(&elev_));
        return;
    }
    if (controller_.RequestTableUpdated()) { 
        Event(controller_.FsmTableUpdate(&elev_));
        return;
    }
    if (controller_.DoorTimer()->Expired()) {
        Event(controller_.FsmDoorTimeout(&elev_));
        return;
    }

};

void ElevatorNode::StepPeers() {
    const int n = node_id_;

    UpdateOrderMatrixFromButtonSignals();
    controller_.SetRequests(peers_.Orders()->ToBoolTable(n));

    elev_.State()->IncrementVersion();
    peers_.State(n)->CopyFrom(elev_.State());
    peers_.Step(n);
}

void ElevatorNode::RxPacketProcessing(network::NetworkPacket packet) {
    if (packet.ID() == node_id_) return;

    std::lock_guard<std::mutex> lock(peers_mutex_);

    peers_.State(packet.ID())->OnUpdate(*packet.State());
    peers_.Orders()->Join(*packet.Orders());
    
    // Preserve cab button status so it can be restored after
    // a node potentially crashes
    for (int f = 0; f < kFloors; f++) {
        elev::ordersync::Order cab = *packet.Orders()->Order(f, (int)BtnType::Cab);
        peers_.CabButtonOrder(packet.ID(), f)->OnUpdate(cab);
    }
}

void ElevatorNode::Init() {
    elev_.Init();
    elev::common::Print("[NODE] Intitialized");
}

void ElevatorNode::Stop() {
    running_.store(false);
}

elev::network::NetworkPacket ElevatorNode::TxPacketCopy() {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    const int n = node_id_;

    elev::network::NetworkPacket packet;
    
    packet.Init(peers_.Orders(), peers_.State(n), peers_.CabButtonOrders());

    return packet; 
}


void ElevatorNode::Event(ButtonFlags b2c) {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    const int n = node_id_;

    // Increment state after fsm event
    elev_.State()->IncrementVersion();

    // Set clear orders on peers
    ButtonFlags zeros{};
    if (b2c != zeros) {
        peers_.ClearOrders(n, elev_.State()->Floor(), b2c);
    }

    // Sync requests
    controller_.SetRequests(peers_.Orders()->ToBoolTable(n));
    elev_.State()->SetRequests(peers_.Orders()->ToBoolTable(n));

    // Sync ElevatorState
    peers_.State(n)->CopyFrom(elev_.State());
}

int ElevatorNode::Id() {
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
                peers_.Orders()->Order(f, b)->OnRequest(n);
            }
        }
    }
}

void ElevatorNode::SetButtonLamps() {
    using namespace elev::common;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            bool light = controller_.Requests().Value(f, b);
            elev_.SetButtonLamp(f, (BtnType)b, light);
        }
    }
}

} // namespace elev::node
