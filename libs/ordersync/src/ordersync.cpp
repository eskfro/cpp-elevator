#include "common/config.hpp"
#include "common/types.hpp"
#include <array>

#include <cstdint>
#include <ordersync/ordersync.hpp>

using namespace elev::common;
using namespace elev::config;


namespace elev::ordersync {
    
OrderTable* OrderMatrix::Table(int elevID) {
    return &matrix_[elevID];
}


ordersync::Order OrderTable::Order(int floor, int btn) {
    return table_[floor][btn];
}


void OrderTable::SetOrder(int floor, int btn, ordersync::Order order) {
    table_[floor][(int)btn].SetStatus(order.Status());
    table_[floor][(int)btn].SetVersion(order.Version());
}


void OrderTable::SetFromButtonFlags(int floor, ButtonFlags b2c) {
    for (int b = 0; b < N_BUTTONS; b++) {
        if (b2c.at(b)) {
            table_[floor][b].OnClear();
        }
    }
}   


    
std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> OrderTable::ToBoolTable(){
    using namespace elev::common;
    std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> result{};

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {            
            result[f][b] = (table_[f][b].Status() == OrderStatus::CONFIRMED) ? true : false;
        }
    }
    return result;
}


void OrderTable::Join(OrderTable rcv) {
    // p2p schema for distributing orders
    // OrderStatus: NONE, REQUESTED, CONFIRMED, CLEAR
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {

            // We dont control the incoming nodes cab orders
            if ((BtnType)b == BtnType::CAB) {
                table_[f][b] = rcv.Order(f, b);
                continue;
            }
            table_[f][b].OnUpdate(rcv.Order(f, b));
        }
    }
}

void Order::OnUpdate(Order rcv) {
    if (rcv.Version() > version_) {
        version_ = rcv.Version();
        status_  = rcv.Status();
    } 
    else if (rcv.Version() == version_ && (uint8_t)(rcv.Status()) > (uint8_t)(status_)) {
        // Adopt status and bump version so the state advancement is explicit
        version_ = rcv.Version() + 1; 
        status_  = rcv.Status();
    }
}

void Order::OnRequest() {
    if (status_ == OrderStatus::NONE) {
        status_ = OrderStatus::REQUESTED;
        version_++;
    }
}

void Order::OnConfirm() {
    if (status_ == OrderStatus::REQUESTED) {
        status_ = OrderStatus::CONFIRMED;
        version_++;
    }
}

void Order::OnClear() {
    if (status_ == OrderStatus::CONFIRMED || status_ == OrderStatus::REQUESTED) {
        status_ = OrderStatus::CLEAR;
        version_++;
    }
}

void Order::OnReset() {
    if (status_ == OrderStatus::CLEAR) {
        status_ = OrderStatus::NONE;
        version_++;
    }
}


}