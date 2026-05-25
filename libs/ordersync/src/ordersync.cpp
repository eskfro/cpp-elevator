#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::ordersync {
    
    
OrderMatrix::OrderMatrix() {
    clearTable();
}


void OrderMatrix::clearTable() {
    using namespace elev::config;
    using namespace elev::common;

    for (int e = 0; e < N_ELEVS; e++) {
        for (int f = 0; f < N_FLOORS; f++) {
            for (int b = 0; b < N_BUTTONS; b++) {
                this->table[e][f][b] = OrderStatus::NONE;
            }
        }
    }
}


elev::common::OrderStatus OrderMatrix::getStatusAt(int elevID, int floor, BtnType btn) {
    return this->table[elevID][floor][(int)btn];
}

void OrderMatrix::setStatusAt(int elevID, int floor, BtnType btn, OrderStatus status) {
    this->table[elevID][floor][(int)btn] = status;
}
    
}


