#pragma once

// Libs
#include <array>
#include <common/config.hpp>
#include <common/types.hpp>

using namespace elev::common;
using namespace elev::config;

namespace elev::ordersync {

class Order {
public:
    Order() = default;

    // Set
    void Observe(int elev_id);
    void SetAssignedId(int assigned_id);

    // Get
    uint64_t Version() const { return version_; }
    OrderStatus Status() const { return status_; }
    int AssignedId() const { return assigned_id_; }
    uint32_t ObservedMask() const { return observed_mask_; }
    [[nodiscard]] bool ObservedBy(int elev_id) const;
    [[nodiscard]] bool Valid() const;

    // State machine
    void OnUpdate(const Order& rcv);
    void OnRequest(int elev_id);
    void OnConfirm(int elev_id);
    void OnClear();
    void OnReset();
    void OnRevoke();
    void OnReassignment(int assigned_id);

private:
    int assigned_id_{-1};
    uint32_t observed_mask_{};
    OrderStatus status_{};
    uint64_t version_{};
};

using CabOrderTable = std::array<std::array<Order, kFloors>, kElevs>;

class OrderTable {
public:
    OrderTable() = default;

    // Set
    void Join(const OrderTable& rcv);

    // Get
    ordersync::Order* Order(int floor, int btn);
    const ordersync::Order* Order(int floor, int btn) const;
    BoolTable ToBoolTable(int elev_id) const;
    [[nodiscard]] bool Valid() const;

private:
    std::array<std::array<ordersync::Order, kButtons>, kFloors> table_{};
};

}  // namespace elev::ordersync