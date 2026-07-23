#include "common/config.hpp"
#include "common/types.hpp"
#include <array>

#include <ordersync/ordersync.hpp>

using namespace elev::common;
using namespace elev::config;


namespace elev::ordersync {
    
OrderTable* OrderMatrix::Table(int elevID) {
    return &matrix_[elevID];
}


OrderStatus OrderTable::Status(int floor, BtnType btn) {
    return table_[floor][(int)btn];
}


void OrderTable::SetStatus(int floor, BtnType btn, OrderStatus status) {
    table_[floor][(int)btn] = status;
}


void OrderTable::SetFromButtonFlags(int floor, ButtonFlags b2c) {
    for (int b = 0; b < N_BUTTONS; b++) {
        if (b2c.at(b)) {
            // TODO:
            // Distributed logic
            table_[floor][b] = OrderStatus::NONE; 
        }
    }
}


void OrderTable::ClearTable() {
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            table_[f][b] = OrderStatus::NONE;
        }
    }
}

    
OrderMatrix::OrderMatrix() {
    ClearMatrix();
}


void OrderMatrix::ClearMatrix() {
    for (int e = 0; e < N_ELEVS; e++) {
        matrix_[e].ClearTable();
    }
}
    
std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> OrderTable::ToBoolTable(){
    using namespace elev::common;
    std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> result{0};

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {            
            result[f][b] = (table_[f][b] == OrderStatus::CONFIRMED) ? true : false;
        }
    }
    return result;
}


}