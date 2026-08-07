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

        uint64_t Version() { return version_; }
        OrderStatus Status() { return status_; }

        void SetStatus(OrderStatus status) { status_ = status; }
        void SetVersion(uint64_t version) { version_ = version; }

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

        // Get
        ordersync::Order* Order(int floor, int btn);
        std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> ToBoolTable();

        // Set
        void SetOrder(int floor, int btn, ordersync::Order order);
        void SetFromButtonFlags(int floor, ButtonFlags b2c);
        void Join(OrderTable rcv);

    private:
        ordersync::Order table_[N_FLOORS][N_BUTTONS];
};


class OrderMatrix {
    public:
        OrderMatrix() = default;
        OrderTable* Table(int elevID);

    private:
        OrderTable matrix_[N_ELEVS];
};

} // namespace elev::ordersync