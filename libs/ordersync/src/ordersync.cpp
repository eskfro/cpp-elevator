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


OrderStatus OrderTable::Status(int floor, int btn) {
    return table_[floor][btn];
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
    std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> result{};

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {            
            result[f][b] = (table_[f][b] == OrderStatus::CONFIRMED) ? true : false;
        }
    }
    return result;
}

constexpr OrderStatus CabOrderTransition[4][4] {

// rcv:      NONE            REQUESTED               CONFIRMED               CLEAR                  this:
    {OrderStatus::NONE,      OrderStatus::REQUESTED, OrderStatus::CONFIRMED, OrderStatus::NONE}, // NONE
    {OrderStatus::REQUESTED, OrderStatus::REQUESTED, OrderStatus::CONFIRMED, OrderStatus::NONE}, // REQUESTED
    {OrderStatus::CONFIRMED, OrderStatus::CONFIRMED, OrderStatus::CONFIRMED, OrderStatus::NONE}, // CONFIRMED
    {OrderStatus::CLEAR,     OrderStatus::CLEAR,     OrderStatus::CLEAR,     OrderStatus::CLEAR} // CLEAR
};

void OrderTable::Join(OrderTable rcv) {
    // p2p schema for distributing orders
    // OrderStatus: NONE, REQUESTED, CONFIRMED, CLEAR
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            OrderStatus status = table_[f][b];

            // We dont control the incoming nodes cab orders
            if ((BtnType)b == BtnType::CAB) {
                table_[f][b] = rcv.Status(f, b);
                continue;
            }


            // Will add CabOrderTransition here ...
            

        }
    }
}


}