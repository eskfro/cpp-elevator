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

namespace elev::network {

Peers::Peers() :
     num_elevs_(0) {}

void Peers::Step(int elev_id) {

     // All elevators should also say requested for that order: (e, f, b).
     // Each elevator has their own copy of all the OrderTables for each elevator, and 
     //      also a copy of what all the other elevator think about eachother. Therefore  
     //      this function need to use all that data to determine if it should set 
     //      OrderStatus::CONFIRMED indicating that i take the order. The plan is to make
     //      it so that the original order (e, f, b) is still marked OrderStatus::REQUESTED
     //      and then use a IsRequestedByAll(f, b) function and then decide if someone should
     //      take the order again if somehow the order disappears because of a network failure etc...
     ConfirmOrders(elev_id);
}

// Confirm orders using the node's table
void Peers::ConfirmOrders(int node_id) {
     using namespace elev::common;
     int n = node_id;

     ordersync::OrderMatrix* matrix = &all_matrices_[node_id];

     // Iterate over all orders (f, b)
     for (int f = 0; f < N_FLOORS; f++) {
          for (int b = 0; b < N_BUTTONS; b++) {

               // Cab
               if ((BtnType)b == BtnType::CAB) {
                    matrix->Table(n)->Order(f, b)->OnConfirm();
                    continue;
               }

               // Hall
               if (!RequestedByAll(n, f, b)) continue;
               int best_elev_id = ElevatorWithLowestCost(f, b);
               matrix->Table(best_elev_id)->Order(f, b)->OnConfirm();
          }
     }
}

// Checks only this node's table
// We might have one dimension too many, but we keep it for now
// might be useful later? 
bool Peers::RequestedByAll(int node_id, int floor, int btn) {
     int n = node_id;
     using namespace elev::common;
     for (int e = 0; e < N_ELEVS; e++) {
          OrderStatus status = all_matrices_[n].Table(e)->Order(floor, btn)->Status();
          if (status != OrderStatus::REQUESTED) {
               return false;
          }
     }
     return true;
}

int Peers::ElevatorWithLowestCost(int floor, int btn) {
     using namespace elev::common;
     int best_elev_id = 0;
     int lowest_cost = std::numeric_limits<int>::max();

     for (int e = 0; e < N_ELEVS; e++) {
          elevator::ElevatorState state = all_states_[e];
          int cost = 0;

          if (!state.Active()) continue;

          cost += std::abs(state.Floor() - floor) * kPenaltyFloorDiff;

          if (state.Obstruction()) cost += kPenaltyObstruction;

          for (int f = 0; f < N_FLOORS; f++) {
               for (int b=0; b<N_BUTTONS; b++) { 
                    if (state.Requests().at(f).at(b)) {
                         cost += kPenaltyPerOrder;
                    }
               }
          }

          if (state.DoorOpen()) cost += kPenaltyDoorOpen;

          // TODO: direction penalty

          if (cost < lowest_cost) {
               lowest_cost = cost;
               best_elev_id = e;
          }
     }
     return best_elev_id;
}

elev::elevator::ElevatorState* Peers::State(int elev_id) {
     return &all_states_[elev_id];
}

elev::ordersync::OrderMatrix* Peers::Matrix(int elev_id) {
     return &all_matrices_[elev_id];
}

int Peers::NumElevs() {
     return num_elevs_;
}

void Peers::SetNumElevs(int num_elevs) {
     num_elevs_ = num_elevs;
}

void Peers::SetMatrix(int elev_id, elev::ordersync::OrderMatrix matrix) {
     all_matrices_[elev_id] = matrix;
}

void Peers::SetClearOrders(int elev_id, int floor, ButtonFlags b2c) {
    all_matrices_[elev_id].Table(elev_id)->SetFromButtonFlags(floor, b2c);
}

void Peers::SetState(int elev_id, elev::elevator::ElevatorState state) {
     all_states_[elev_id] = state;
}

} // namespace elev::network

