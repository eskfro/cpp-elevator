#include <array>
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
#include <utility>
#include <algorithm>

#include "common/config.hpp"
#include "common/utils.hpp"
#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"

namespace elev::network {

void Peers::Step() {
    MonitorWatchdogTimers();
    MonitorFault();
    UpdateNumElevs();
    ObserveOrders();
    ConfirmHallOrders();
    ResetHallOrders();
    MonitorHallOrderTimers();
    ControlHallOrderTimers();
}

// Mark this node as having seen a hall order, so ObservedByAll can converge
void Peers::ObserveOrders() {
    using namespace elev::common;
    const int n = node_id_;

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
    const int n = node_id_;
    using namespace elev::common;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab || 
                !ObservedByAll(f, b) || 
                orders_.Order(f, b)->Status() != OrderStatus::Requested) continue;
            
            // pair: (cost, elev_id)
            std::array<std::pair<int, int>, kElevs> costs = CalculateElevatorCosts(f, b);
            int best_elev_id = costs[0].second;

            if (costs[0].first == std::numeric_limits<int>::max()) {
                common::PrintError("[Peers] No active elevators in all_elevs_");
                continue;
            }

            orders_.Order(f, b)->OnConfirm(best_elev_id);

            if (best_elev_id == n) {
                order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
            }
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

std::array<std::pair<int, int>, kElevs> Peers::CalculateElevatorCosts(int floor, int btn) {
    using namespace elev::common;

    std::array<std::pair<int, int>, kElevs> costs{};

    for (int e = 0; e < kElevs; e++) {
        elevator::ElevatorState state = all_states_[e];
        int cost = 0;

        if (!state.Active()) {
            costs[e] = {std::numeric_limits<int>::max(), e};
            continue;
        }

        cost += std::abs(state.Floor() - floor) * kPenaltyFloorDiff;

        if (state.Obstruction()) cost += kPenaltyObstruction;
        if (state.Fault()) cost += kPenaltyFault;
        if (state.Stopped()) cost += kPenaltyStopped;

        auto reqs = state.Requests();
        for (int f = 0; f < kFloors; f++) {
            for (int b = 0; b < kButtons; b++) {
                if (reqs.at(f).at(b)) {
                    cost += kPenaltyPerOrder;
                }
            }
        }

        if (state.DoorOpen()) cost += kPenaltyDoorOpen;

        // Directional penalties
        bool moving = state.MotorDir() != MotorDir::Stop;
        bool order_below = floor < state.Floor();
        bool order_above = floor > state.Floor();
        bool order_here = floor == state.Floor();
        bool going_up = state.Inertia() == Inertia::Up;
        bool going_down = state.Inertia() == Inertia::Down;

        bool wrong_dir_1 =
            (moving && order_below && going_up) ||
            (moving && order_above && going_down);
        if (wrong_dir_1) cost += kPenaltyWrongDir;

        bool wrong_dir_2 = 
            (order_here && moving);
        if (wrong_dir_2) cost += kPenaltyWrongDir;

        costs[e] = {cost, e};
    }

    std::sort(costs.begin(), costs.end());

    return costs;
}

elev::elevator::ElevatorState* Peers::State(int elev_id) {
    assert(elev_id >= 0 && elev_id < kElevs);
    return &all_states_[elev_id];
}

elev::ordersync::OrderTable* Peers::Orders() { return &orders_; }

int Peers::NumElevs() { return num_elevs_; }

void Peers::ClearOrders(int floor, ButtonFlags b2c) {
    const int n = node_id_;
    for (int b = 0; b < kButtons; b++) {
        if (!b2c.at(b)) continue;

        // Cab
        if ((BtnType)b == BtnType::Cab) {
            orders_.Order(floor, b)->OnClear();
            orders_.Order(floor, b)->OnReset();
            cab_button_orders_[n][floor].OnClear();
            cab_button_orders_[n][floor].OnReset();
            continue;
        }

        // Hall
        if (orders_.Order(floor, b)->AssignedId() == n) {
            orders_.Order(floor, b)->OnClear();
            order_timers_.Timer(floor, b)->Stop();
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
    const int n = node_id_;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;

            if (order_timers_.Timer(f, b)->Expired() == false) continue;

            order_timers_.Timer(f, b)->Stop();

            // Order already cleared
            if (orders_.Order(f, b)->Status() != OrderStatus::Confirmed) continue;

            ReassignHallOrder(f, b);

            if (orders_.Order(f, b)->AssignedId() == n) {   
                order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
            }
        }
    }
}

void Peers::ReassignHallOrder(int floor, int btn) {
    std::string msg = 
        std::string("[PEERS] Order (") +
        std::to_string(floor) + ", " + std::to_string(btn) + 
        std::string(") timed out - reassigning");
    elev::common::PrintError(msg);

    int prev_assignee = orders_.Order(floor, btn)->AssignedId();
    assert(prev_assignee >= 0 && prev_assignee < kElevs && "prev_assignee in range");

    // Reassign to self
    if (num_elevs_ == 1) {
        orders_.Order(floor, btn)->OnReassignment(prev_assignee);
        return;
    }
    
    // Reassigning rules for > 1 elev
    bool prev_assigne_usable = all_states_[prev_assignee].Usable();
    int new_assignee = -1;

    // pair: (cost, elev_id)
    std::array<std::pair<int, int>, kElevs> costs = CalculateElevatorCosts(floor, btn);

    // Return the elevid with lowest cost that is not prev_assigne
    if (!prev_assigne_usable) {
        for (int i = 0; i < kElevs; i++) {
            int cost = costs[i].first;
            int e = costs[i].second;
            if (e != prev_assignee && cost != std::numeric_limits<int>::max()) {
                new_assignee = e;
                break;
            }
        }
    // Assign to another elev only if usable    
    } else {
        int best_other_usable = -1;
        for (int i = 0; i < kElevs; i++) {
            int cost = costs[i].first;
            int e = costs[i].second;
            if (e != prev_assignee && cost != std::numeric_limits<int>::max() && all_states_[e].Usable()) {
                best_other_usable = e;
                break;
            }
        } 
        if (best_other_usable != -1) {
            new_assignee = best_other_usable;
        } else {
            new_assignee = prev_assignee;
        }
    }
    
    // No new assigne found - just reassign
    if (new_assignee == -1) {
        if (all_states_.at(prev_assignee).Active()) {
            new_assignee = prev_assignee;
        } else {
            elev::common::Abort("[PEERS] Could not find new assigne during reassignment");
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
    const int n = node_id_;
    /*
    Reassigns hall orders for elev_id
    */
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;
            if (orders_.Order(f, b)->Status() != OrderStatus::Confirmed) continue;
            if (orders_.Order(f, b)->AssignedId() != elev_id) continue;

            ReassignHallOrder(f, b);

            if (orders_.Order(f, b)->AssignedId() == n) {
                order_timers_.Timer(f, b)->Start(kReassignOrderTimeMs);
            }
        }
    }
}

void Peers::ControlHallOrderTimers() {
    /*
    Check if this node should start or stop hall order timers
    */
    const int n = node_id_;
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if ((BtnType)b == BtnType::Cab) continue;

            elev::common::Timer* timer = order_timers_.Timer(f, b);
            elev::ordersync::Order* order = orders_.Order(f, b);

            // Start
            bool should_start_timer = 
                order->Status() == OrderStatus::Confirmed &&
                timer->Active() == false && 
                order->AssignedId() == n;
            if (should_start_timer) {
                timer->Start(kReassignOrderTimeMs);
                continue;
            }
            // Stop
            bool should_stop_timer = 
                timer->Active() == true &&
                (order->Status() != OrderStatus::Confirmed || order->AssignedId() != n);
            if (should_stop_timer) {
                timer->Stop();
                continue;
            }
        }
    }
}

void Peers::Init(int node_id) {
    node_id_ = node_id;
}

}  // namespace elev::network
