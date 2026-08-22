#include "common/config.hpp"
#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <network/peers.hpp>
#include <common/types.hpp>
#include <limits.h>
#include <stdatomic.h>
#include <assert.h>

namespace elev::network {

void Peers::Step(int node_id) {
     UpdateNumElevs();
     ObserveOrders(node_id);
     ConfirmHallOrders(node_id);
     ResetHallOrders(node_id);
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
void Peers::ConfirmHallOrders(int node_id) {
     using namespace elev::common;
     const int n = node_id;

     // Iterate over all orders (f, b)
     for (int f = 0; f < kFloors; f++) {
          for (int b = 0; b < kButtons; b++) {

               if ((BtnType)b == BtnType::Cab) continue;

               if (!ObservedByAll(f, b)) continue;

               int best_elev_id = ElevatorWithLowestCost(f, b);
               if (best_elev_id == -1) {
                    common::PrintError("[Peers] No active elevators in all_elevs_");
                    continue;
               }

               orders_.Order(f, b)->OnConfirm(best_elev_id);
          }
     }
}

void Peers::ResetHallOrders(int node_id) {
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

          for (int f = 0; f < kFloors; f++) {
               for (int b=0; b<kButtons; b++) { 
                    if (state.Requests().at(f).at(b)) {
                         cost += kPenaltyPerOrder;
                    }
               }
          }

          if (state.DoorOpen()) cost += kPenaltyDoorOpen;
          
          // Very simple directional penalty
          bool order_below = floor < state.Floor();
          bool order_above = floor > state.Floor();
          bool wrong_dir = 
               (order_below && state.Inertia() == Inertia::Up) || 
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

elev::ordersync::OrderTable* Peers::Orders() {
     return &orders_;
}

int Peers::NumElevs() {
     return num_elevs_;
}

void Peers::ClearOrders(int node_id, int floor, ButtonFlags b2c) {
     const int n = node_id;
     for (int b = 0; b < kButtons; b++) {
          
          if (!b2c.at(b)) continue;

          bool is_cab = (BtnType)b == BtnType::Cab;

          orders_.Order(floor, b)->OnClear();
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

std::array<std::array<elev::ordersync::Order, kFloors>, kElevs>* Peers::CabButtonOrders() {
     return &cab_button_orders_;
}

} // namespace elev::network

