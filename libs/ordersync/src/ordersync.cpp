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

ordersync::Order* OrderTable::Order(int floor, int btn) {
    return &table_[floor][btn];
}

void OrderTable::SetOrder(int floor, int btn, ordersync::Order order) {
    table_[floor][(int)btn].SetStatus(order.Status());
    table_[floor][(int)btn].SetVersion(order.Version());
}

void OrderTable::ClearOrders(int floor, ButtonFlags b2c) {
    for (int b = 0; b < kButtons; b++) {
        if (b2c.at(b)) table_[floor][b].OnClear();
    }
}   
    
std::array<std::array<bool, elev::config::kButtons>, config::kFloors> OrderTable::ToBoolTable(){
    using namespace elev::common;
    std::array<std::array<bool, elev::config::kButtons>, config::kFloors> result{};

    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {            
            result[f][b] = (table_[f][b].Status() == OrderStatus::Confirmed) ? true : false;
        }
    }
    return result;
}

void OrderTable::Join(OrderTable rcv) {
    // p2p schema for distributing orders
    // OrderStatus: NONE, REQUESTED, CONFIRMED, CLEAR
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            table_[f][b].OnUpdate(*rcv.Order(f, b));
        }
    }
}

void OrderMatrix::Join(OrderMatrix rcv) {
    for (int e = 0; e < kElevs; e++) {
        matrix_[e].Join(*rcv.Table(e));
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
    if (status_ == OrderStatus::None) {
        status_ = OrderStatus::Requested;
        version_++;
    }
}

void Order::OnConfirm() {
    if (status_ == OrderStatus::Requested) {
        status_ = OrderStatus::Confirmed;
        version_++;
    }
}

void Order::OnClear() {
    if (status_ == OrderStatus::Confirmed) {
        status_ = OrderStatus::Clear;
        version_++;
    }
}

void Order::OnReset() {
    if (status_ == OrderStatus::Clear) {
        status_ = OrderStatus::None;
        version_++;
    }
}

} // namespace elev::ordersync