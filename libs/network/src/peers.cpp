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
     ConfirmOrders(node_id);
     ResetOrders(node_id);
}

// Confirm orders using the node's table
void Peers::ConfirmOrders(int node_id) {
     using namespace elev::common;
     const int n = node_id;

     // Iterate over all orders (f, b)
     for (int f = 0; f < kFloors; f++) {
          for (int b = 0; b < kButtons; b++) {

               // Cab
               if ((BtnType)b == BtnType::Cab) {
                    orders_.Table(n)->Order(f, b)->OnConfirm();
                    continue;
               }

               // Hall
               if (!RequestedByAll(f, b)) continue;

               int best_elev_id = ElevatorWithLowestCost(f, b);
               if (best_elev_id == -1) {
                    common::PrintError("[Peers] No active elevators in all_elevs_");
                    continue;
               }
               orders_.Table(best_elev_id)->Order(f, b)->OnConfirm();
          }
     }
}

bool Peers::RequestedByAll(int floor, int btn) {
     using namespace elev::common;
     for (int e = 0; e < kElevs; e++) {
          if (all_states_[e].Active() == false) continue;

          OrderStatus status = orders_.Table(e)->Order(floor, btn)->Status();
          if (status != OrderStatus::Requested) {
               return false;
          }
     }
     return true;
}

bool Peers::ClearedByAll(int floor, int btn) {
     using namespace elev::common;
     for (int e = 0; e < kElevs; e++) {
          if (all_states_[e].Active() == false) continue;

          OrderStatus status = orders_.Table(e)->Order(floor, btn)->Status();
          if (status != OrderStatus::Clear) {
               return false;
          }
     }
     return true;
}

void Peers::ResetOrders(int node_id) {
     const int n = node_id;
     for (int f = 0; f < kFloors; f++) {
          for (int b = 0; b < kButtons; b++) {
               if (!ClearedByAll(f, b)) continue;

               orders_.Table(n)->Order(f, b)->OnReset();
          }
     }
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

elev::ordersync::OrderMatrix* Peers::Orders() {
     return &orders_;
}

int Peers::NumElevs() {
     return num_elevs_;
}

void Peers::SetNumElevs(int num_elevs) {
     num_elevs_ = num_elevs;
}

void Peers::ClearOrders(int node_id, int floor, ButtonFlags b2c) {
     for (int e = 0; e < kElevs; e++) {
          if (all_states_[e].Active() == false) continue;
          for (int b = 0; b < kButtons; b++) {
               if (b2c.at(b)) {
                    orders_.Table(e)->Order(floor, b)->OnClear();
               }
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

} // namespace elev::network

