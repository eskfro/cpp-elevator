#pragma once

// Libs
#include <array>
#include <common/config.hpp>
#include <common/types.hpp>


using namespace elev::common;
using namespace elev::config;

namespace elev::ordersync {

class OrderTable {
    public:
        OrderTable() = default;

        // Get
        OrderStatus Status(int floor, int btn);
        std::array<std::array<bool, elev::config::N_BUTTONS>, config::N_FLOORS> ToBoolTable();

        // Set
        void SetStatus(int floor, BtnType btn, OrderStatus status);
        void SetFromButtonFlags(int floor, ButtonFlags b2c);
        void Join(OrderTable rcv);

        void ClearTable();

    private:
        OrderStatus table_[N_FLOORS][N_BUTTONS];
};


class OrderMatrix {
    public:
        OrderMatrix();
        void ClearMatrix();
        OrderTable* Table(int elevID);

    private:
        OrderTable matrix_[N_ELEVS];
};

} // namespace elev::ordersync