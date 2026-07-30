#include "common/config.hpp"
#include "elevator/elevator_state.hpp"
#include <cstdint>
#include <mutex>
#include <network/peers.hpp>
#include <common/types.hpp>

namespace elev::network {

Peers::Peers() :
     num_elevs_(0) {}

void Peers::Step(int elev_id) {
     // TODO: Sync all_matrices_[NodeID] matrix according to OrderStatus transitions
     // When a button is pressed on one elevator it becomes OrderStatus::REQUESTED
     SyncMatrix(elev_id);

     // All elevators should also say requested for that order: (e, f, b).
     // Each elevator has their own copy of all the OrderTables for each elevator, and 
     //      also a copy of what all the other elevator think about eachother. Therefore  
     //      this function need to use all that data to determine if it should set 
     //      OrderStatus::CONFIRMED indicating that i take the order. The plan is to make
     //      it so that the original order (e, f, b) is still marked OrderStatus::REQUESTED
     //      and then use a IsRequestedByAll(f, b) function and then decide if someone should
     //      take the order again if somehow the order disappears because of a network failure etc...
     AcceptPotentialOrders(elev_id);
}


void Peers::AcceptPotentialOrders(int elev_id) {

     // Iterate over all orders (f, b)
     for (int f = 0; f < N_FLOORS; f++) {
          for (int b = 0; b < N_BUTTONS; b++) {
               

               if (!RequestedByAll(elev_id, f, (BtnType)b)) {
                    continue;
               }

               for (int e = 0; e < N_ELEVS; e++) {

               }
          }
     }
}


bool Peers::RequestedByAll(int elev_id, int floor, elev::common::BtnType btn) {
     return false;
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

uint64_t Peers::Version(int elev_id) {
     return versions_[elev_id];
}

void Peers::SetNumElevs(int num_elevs) {
     num_elevs_ = num_elevs;
}

void Peers::SetMatrix(int elevID, elev::ordersync::OrderMatrix matrix) {
     all_matrices_[elevID] = matrix;
}

void Peers::SetClearOrders(int elevID, int floor, ButtonFlags b2c) {
    all_matrices_[elevID].Table(elevID)->SetFromButtonFlags(floor, b2c);
}

void Peers::IncrementVersion(int elev_id) {
     versions_[elev_id]++;
}

} // namespace elev::network

