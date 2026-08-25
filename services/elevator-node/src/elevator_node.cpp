#include <chrono>
#include <mutex>
#include <ratio>
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

bool ElevatorNode::Running() { return running_.load(); }

ElevatorNode::ElevatorNode(int id, std::string ip) :
    node_id_(id),
    running_(true),
    elev_(id, ip) {}

void ElevatorNode::Step() {
    // Maybe implement some state machine here later
    switch (service_state_) {
    case elev::node::ServiceState::Startup:
    case elev::node::ServiceState::Running:
    case elev::node::ServiceState::Stopped:
        elev_.Step();
        {
            // --- LOCKED ---
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
        if (controller_.FloorTimer()->Expired()) {
            Event(controller_.FsmFloorTimeout(&elev_));
            return;
        }
    }
};

// All these operations are locked with mutex
void ElevatorNode::StepPeers() {
    const int n = node_id_;

    // Button presses
    RegisterButtonSignals();
    controller_.SetRequests(peers_.Orders()->ToBoolTable(n));

    // State update
    elev_.State()->IncrementVersion();
    peers_.State(n)->CopyFrom(elev_.State());

    // Peer logic step
    peers_.Step(n);
}

void ElevatorNode::RxPacketProcessing(network::NetworkPacket packet) {
    if (packet.Id() == node_id_) return;

    const int p = packet.Id();
    const int n = node_id_;

    // --- LOCKED ---
    std::lock_guard<std::mutex> lock(peers_mutex_); 

    peers_.UpdateWatchdogTimer(p);
    peers_.State(p)->OnUpdate(*packet.State());
    peers_.Orders()->Join(*packet.Orders());
    peers_.ControlHallOrderTimers();

    // Cab order preservation join
    for (int f = 0; f < kFloors; f++) {
        elev::ordersync::Order cab_packet = *packet.Orders()->Order(f, (int)BtnType::Cab);
        elev::ordersync::Order cab_node = packet.CabButtonOrders()->at(n).at(f);

        peers_.CabButtonOrder(p, f)->OnUpdate(cab_packet);
        peers_.CabButtonOrder(n, f)->OnUpdate(cab_node);
        peers_.Orders()->Order(f, (int)BtnType::Cab)->OnUpdate(cab_node);
    }
}

void ElevatorNode::Init() {
    elev_.Init();
    /*
    * Delay startup to ensure the peers' watchdog timers have timed out
    * This is not an issue when manually restarting the nodes, but could potentially
    * become one if process pairs are implemented in the future.
    */
    std::this_thread::sleep_for(std::chrono::milliseconds(kStartupTimeMs));

    elev::common::Print("[NODE] Intitialized");
}

void ElevatorNode::Stop() { running_.store(false); }

elev::network::NetworkPacket ElevatorNode::TxPacketCopy() {
    // --- LOCKED ---
    std::lock_guard<std::mutex> lock(peers_mutex_);

    const int n = node_id_;
    
    elev::network::NetworkPacket packet;
    packet.Init(peers_.Orders(), peers_.State(n), peers_.CabButtonOrders());
    return packet;
}

void ElevatorNode::Event(ButtonFlags b2c) {
    std::lock_guard<std::mutex> lock(peers_mutex_);
    const int n = node_id_;

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

int ElevatorNode::Id() { return node_id_; }

// Polls BtnSignals and set status at OrderMatrix orders
void ElevatorNode::RegisterButtonSignals() {
    using namespace elev::common;
    const int n = node_id_;

    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            bool is_cab = (BtnType)b == BtnType::Cab;
            bool btn_pressed = elev_.Buttons()->Button(f, (BtnType)b)->Pressed();
            if (!btn_pressed) continue;
            
            PrintBtnPress(n, f, (BtnType)b);
            
            // Cab
            if (is_cab) {
                peers_.Orders()->Order(f, b)->OnRequest(n);
                peers_.Orders()->Order(f, b)->OnConfirm(n);
                peers_.CabButtonOrder(n, f)->OnRequest(n);
                peers_.CabButtonOrder(n, f)->OnConfirm(n);

            // Hall
            } else {
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

}  // namespace elev::node
