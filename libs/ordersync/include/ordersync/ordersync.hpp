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
        void SetStatus(OrderStatus status) { status_ = status; }
        void SetVersion(uint64_t version) { version_ = version; }

        // Get
        uint64_t Version() { return version_; }
        OrderStatus Status() { return status_; }

        // State machine
        void OnUpdate(Order rcv);
        void OnRequest();
        void OnConfirm();
        void OnClear();
        void OnReset();
        
    private:
        OrderStatus status_{};
        uint64_t version_{};
};

class OrderTable {
    public:
        OrderTable() = default;

        // Set
        void SetOrder(int floor, int btn, ordersync::Order order);
        void ClearOrders(int floor, ButtonFlags b2c);
        void Join(OrderTable rcv);

        // Get
        ordersync::Order* Order(int floor, int btn);
        std::array<std::array<bool, kButtons>, kFloors> ToBoolTable();

    private:
        std::array<std::array<ordersync::Order, kButtons>, kFloors> table_{};
};

class OrderMatrix {
    public:
        OrderMatrix() = default;
        
        OrderTable* Table(int elevID);

        void Join(OrderMatrix rcv);

    private:
        std::array<OrderTable, kElevs> matrix_{};
};

} // namespace elev::ordersync