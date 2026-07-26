#include <network/peers.hpp>
#include <common/types.hpp>

namespace elev::network {

Peers::Peers() : num_elevs_{0} {}



int Peers::NumElevs() {
     return num_elevs_;
}


void Peers::SetNumElevs(int num_elevs) {
     num_elevs_ = num_elevs;
}


elev::ordersync::OrderMatrix* Peers::Matrix(int elevID) {
     return &all_matrices_[elevID];
}


void Peers::SetMatrix(int elevID, elev::ordersync::OrderMatrix matrix) {
     all_matrices_[elevID] = matrix;
}

void Peers::SetClearOrders(int elevID, int floor, ButtonFlags b2c) {
    all_matrices_[elevID].Table(elevID)->SetFromButtonFlags(floor, b2c);
}

}