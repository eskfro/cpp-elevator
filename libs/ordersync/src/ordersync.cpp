#include <array>
#include <cstdint>
#include <ordersync/ordersync.hpp>

#include "common/config.hpp"
#include "common/types.hpp"

using namespace elev::common;
using namespace elev::config;

namespace elev::ordersync {

ordersync::Order* OrderTable::Order(int floor, int btn) {
    return &table_[floor][btn];
}

const ordersync::Order* OrderTable::Order(int floor, int btn) const {
    return &table_[floor][btn];
}

BoolTable OrderTable::ToBoolTable(int elev_id) const {
    BoolTable result{};

    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            result[f][b] = (table_[f][b].Status() == OrderStatus::Confirmed) &&
                           (table_[f][b].AssignedId() == elev_id);
        }
    }
    return result;
}

bool OrderTable::Valid() const {
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            if (!table_[f][b].Valid()) return false;
        }
    }
    return true;
}

void OrderTable::Join(const OrderTable& rcv) {
    // p2p schema for distributing orders
    // OrderStatus: NONE, REQUESTED, CONFIRMED, CLEAR
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            // Cab buttons are local
            if ((BtnType)b == BtnType::Cab) continue;

            table_[f][b].OnUpdate(*rcv.Order(f, b));
        }
    }
}

bool Order::Valid() const {
    if (assigned_id_ < -1 || assigned_id_ >= kElevs) return false;
    if (static_cast<std::uint8_t>(status_) >
        static_cast<std::uint8_t>(OrderStatus::Clear)) return false;
    if (observed_mask_ & ~((1u << kElevs) - 1u)) return false;
    return true;
}

void Order::OnUpdate(const Order& rcv) {
    if (rcv.Version() > version_) {
        version_ = rcv.Version();
        status_ = rcv.Status();
        assigned_id_ = rcv.AssignedId();
        observed_mask_ = rcv.ObservedMask();
        return;
    } 

    if (rcv.Version() == version_) {
        observed_mask_ |= rcv.ObservedMask();

        if ((uint8_t)(rcv.Status()) > (uint8_t)(status_)) {
            status_ = rcv.Status();
            assigned_id_ = rcv.AssignedId();
            return;
        }
        // Both sides confirmed the same order concurrently
        if (status_ == OrderStatus::Confirmed &&
                rcv.Status() == OrderStatus::Confirmed &&
                rcv.AssignedId() < assigned_id_) {
            assigned_id_ = rcv.AssignedId();
            return;
        }
    }
}

void Order::Observe(int elev_id) { observed_mask_ |= (1u << elev_id); }

bool Order::ObservedBy(int elev_id) const { return observed_mask_ & (1u << elev_id); }

void Order::OnRequest(int elev_id) {
    if (status_ == OrderStatus::None || status_ == OrderStatus::Clear) {
        status_ = OrderStatus::Requested;
        version_++;
        observed_mask_ = 0;
        Observe(elev_id);
    }
}

void Order::OnConfirm(int elev_id) {
    if (status_ == OrderStatus::Requested) {
        status_ = OrderStatus::Confirmed;
        assigned_id_ = elev_id;
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

void Order::OnRevoke() {
    if (status_ == OrderStatus::Confirmed) {
        status_ = OrderStatus::Requested;
        version_++;
    }
}

void Order::SetAssignedId(int assigned_id) {
    assigned_id_ = assigned_id;
}

void Order::OnReassignment(int assigned_id) {
    assigned_id_ = assigned_id;
    version_++;    
}

}  // namespace elev::ordersync