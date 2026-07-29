#include "elevator/elevator_state.hpp"
#include <network/peers.hpp>
#include <common/types.hpp>

namespace elev::network {

Peers::Peers() :
     num_elevs_(0) {}


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


void Peers::SetMatrix(int elevID, elev::ordersync::OrderMatrix matrix) {
     all_matrices_[elevID] = matrix;
}

void Peers::SetClearOrders(int elevID, int floor, ButtonFlags b2c) {
    all_matrices_[elevID].Table(elevID)->SetFromButtonFlags(floor, b2c);
}

}