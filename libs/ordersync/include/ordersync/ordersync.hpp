#pragma once

// Libs
#include <common/config.hpp>
#include <common/types.hpp>

using namespace elev::common;
using namespace elev::config;

namespace elev::ordersync {

class OrderMatrix {
    private:
        OrderStatus table[N_ELEVS][N_FLOORS][N_BUTTONS];

    public:
        OrderMatrix();
        ~OrderMatrix();
        void clearTable();

        OrderStatus getStatusAt(int elevID, int floor, BtnType btn);
        void setStatusAt(int elevID, int floor, BtnType btn, OrderStatus status);
        

};

}