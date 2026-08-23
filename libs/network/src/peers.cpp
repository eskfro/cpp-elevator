#include <assert.h>
#include <limits.h>
#include <stdatomic.h>

#include <common/types.hpp>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <network/peers.hpp>
#include <string>

#include "common/config.hpp"
#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"

namespace elev::network {

void Peers::Step(int node_id) {
    MonitorWatchdogTimers();
    MonitorFault();
    UpdateNumElevs();
    ObserveOrders(node_id);
    ConfirmHallOrders();
    ResetHallOrders();
    MonitorHallOrderTimers();
}

// Mark this node as having seen a hall order, so ObservedByAll can converge
void Peers::ObserveOrders(int node_id) {
    using namespace elev::common;
    const int n = node_id;

    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;

            ordersync::Order* order = orders_.Order(f, b);
            if (order->ObservedBy(n)) continue;
            if (order->Status() == OrderStatus::Requested) {
                order->Observe(n);
            }
        }
    }
}

// Confirm orders using the node's table
void Peers::ConfirmHallOrders() {
    using namespace elev::common;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab || 
                !ObservedByAll(f, b) || 
                orders_.Order(f, b)->Status() != OrderStatus::Requested) continue;
            
            int best_elev_id = ElevatorWithLowestCost(f, b);
            if (best_elev_id == -1) {
                common::PrintError("[Peers] No active elevators in all_elevs_");
                continue;
            }
            orders_.Order(f, b)->OnConfirm(best_elev_id);
            order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
        }
    }
}

void Peers::ResetHallOrders() {
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;
            orders_.Order(f, b)->OnReset();
        }
    }
}

bool Peers::ObservedByAll(int floor, int btn) {
    for (int e = 0; e < kElevs; e++) {
        if (all_states_[e].Active() == false) continue;
        if (orders_.Order(floor, btn)->ObservedBy(e) == false) {
            return false;
        }
    }
    return true;
}

int Peers::ElevatorWithLowestCost(int floor, int btn) {
    using namespace elev::common;
    int best_elev_id = -1;
    int lowest_cost = std::numeric_limits<int>::max();

    for (int e = 0; e < kElevs; e++) {
        elevator::ElevatorState state = all_states_[e];
        int cost = 0;

        if (!state.Active()) continue;

        if (num_elevs_ == 1) return e;

        cost += std::abs(state.Floor() - floor) * kPenaltyFloorDiff;

        if (state.Obstruction()) cost += kPenaltyObstruction;
        if (state.Fault()) cost += kPenaltyFault;
        if (state.Stopped()) cost += kPenaltyStopped;

        for (int f = 0; f < kFloors; f++) {
            for (int b = 0; b < kButtons; b++) {
                if (state.Requests().at(f).at(b)) {
                    cost += kPenaltyPerOrder;
                }
            }
        }

        if (state.DoorOpen()) cost += kPenaltyDoorOpen;

        // Very simple directional penalty
        bool order_below = floor < state.Floor();
        bool order_above = floor > state.Floor();
        bool wrong_dir = (order_below && state.Inertia() == Inertia::Up) ||
                         (order_above && state.Inertia() == Inertia::Down);
        if (wrong_dir) cost += kPenaltyWrongDir;

        if (cost < lowest_cost) {
            lowest_cost = cost;
            best_elev_id = e;
        }
    }
    return best_elev_id;
}

elev::elevator::ElevatorState* Peers::State(int elev_id) {
    assert(elev_id >= 0 && elev_id < kElevs);

    return &all_states_[elev_id];
}

elev::ordersync::OrderTable* Peers::Orders() { return &orders_; }

int Peers::NumElevs() { return num_elevs_; }

void Peers::ClearOrders(int node_id, int floor, ButtonFlags b2c) {
    const int n = node_id;
    for (int b = 0; b < kButtons; b++) {
        if (!b2c.at(b)) continue;

        bool is_cab = (BtnType)b == BtnType::Cab;

        orders_.Order(floor, b)->OnClear();
        order_timers_.Timer(floor, b)->Stop();

        if (is_cab) {
            cab_button_orders_[n][floor].OnClear();
            cab_button_orders_[n][floor].OnReset();
            orders_.Order(floor, b)->OnReset();
        }
    }
}

void Peers::UpdateNumElevs() {
    int count = 0;
    for (int e = 0; e < kElevs; e++) {
        count += (int)all_states_[e].Active();
    }
    num_elevs_ = count;
}

elev::ordersync::Order* Peers::CabButtonOrder(int elev_id, int floor) {
    return &cab_button_orders_[elev_id][floor];
}

elev::ordersync::CabOrderTable* Peers::CabButtonOrders() {
    return &cab_button_orders_;
}

void Peers::MonitorHallOrderTimers() {
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {

            if ((BtnType)b == BtnType::Cab) continue;
            if (!order_timers_.Timer(f, b)->Expired()) continue;

            order_timers_.Timer(f, b)->Stop();

            // Order already cleared
            if (orders_.Order(f, b)->Status() != OrderStatus::Confirmed) continue;

            ReassignHallOrder(f, b);
            order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
        }
    }
}

void Peers::ReassignHallOrder(int floor, int btn) {
    std::string msg = 
        std::string("[PEERS] Order (") +
        std::to_string(floor) + ", " + std::to_string(btn) + 
        std::string(") timed out");
    elev::common::PrintError(msg);

    if (num_elevs_ == 1) return; 

    int prev_assignee = orders_.Order(floor, btn)->AssignedId();
    assert(prev_assignee >= 0 && prev_assignee < kElevs && "prev_assignee in range");
    int new_assignee = -1;

    for (int e = 0; e < kElevs; e++) {
        if (all_states_.at(e).Active() && e != prev_assignee) {
            new_assignee = e;
            break;
        }
    }
    if (new_assignee == -1) {
        if (all_states_.at(prev_assignee).Active()) {
            new_assignee = prev_assignee;
        } else {
            elev::common::PrintError("[PEERS] Could not find new assigne during reassignment");
            abort();
        }
    }
    orders_.Order(floor, btn)->OnReassignment(new_assignee);
}

void Peers::UpdateWatchdogTimer(int elev_id) {
    watchdog_timers_[elev_id].Start(kWatchdogTimeMs);
}

void Peers::MonitorWatchdogTimers() {
    for (int e = 0; e < kElevs; e++) {
        if (watchdog_timers_[e].Expired()) {
            watchdog_timers_[e].Stop();
            all_states_[e].OnWatchdogTimeout();
            elev::common::PrintError("[PEERS] Watchdog timer timed out on elevator " + std::to_string(e));
            ReassignHallOrders(e);
        }
    }
}

void Peers::MonitorFault() {
    for (int e = 0; e < kElevs; e++) {
        if (!all_states_[e].Active()) continue;
        if (all_states_[e].Fault()) {
            ReassignHallOrders(e);
        }
    }
}

void Peers::ReassignHallOrders(int elev_id) {
    /*
        Reassigns hall orders for elev_id
    */
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;
            if (orders_.Order(f, b)->Status() != OrderStatus::Confirmed) continue;

            if (orders_.Order(f, b)->AssignedId() == elev_id) {
                ReassignHallOrder(f, b);
                order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
            }
        }
    }
}

}  // namespace elev::network
