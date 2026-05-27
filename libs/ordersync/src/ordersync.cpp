#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

namespace elev::ordersync {
    
// TODO: make this the = overload function
OrderSlice OrderMatrix::getSliceAt(int elevID) {
    using namespace common;

    OrderSlice slice;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            OrderStatus thisStatus = getStatusAt(elevID, f, (BtnType)b);
            slice.setValueAt(f, (BtnType)b, thisStatus);
        }
    }
    return slice;
}


OrderStatus OrderSlice::getValueAt(int floor, BtnType btn) {
    return table[floor][(int)btn];
}


void OrderSlice::setValueAt(int floor, BtnType btn, OrderStatus status) {
    table[floor][(int)btn] = status;
}

    
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


