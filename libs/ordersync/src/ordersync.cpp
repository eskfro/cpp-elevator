#include <array>

#include <ordersync/ordersync.hpp>

using namespace elev::common;
using namespace elev::config;


namespace elev::ordersync {


void OrderMatrix::setFromButtonFlags(int elevID, int floor, ButtonFlags b2c) {
    for (int b = 0; b < N_BUTTONS; b++) {
        if (b2c[b]) {
            table[elevID][floor][b] = OrderStatus::NONE; // TODO: distr logic
        }
    }   
}
    

OrderSlice OrderMatrix::getSliceAt(int elevID) {
    OrderSlice slice = OrderSlice();
    
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
    for (int e = 0; e < N_ELEVS; e++) {
        for (int f = 0; f < N_FLOORS; f++) {
            for (int b = 0; b < N_BUTTONS; b++) {
                this->table[e][f][b] = OrderStatus::NONE;
            }
        }
    }
}


OrderStatus OrderMatrix::getStatusAt(int elevID, int floor, BtnType btn) {
    return this->table[elevID][floor][(int)btn];
}

void OrderMatrix::setStatusAt(int elevID, int floor, BtnType btn, OrderStatus status) {
    this->table[elevID][floor][(int)btn] = status;
}
    
}


