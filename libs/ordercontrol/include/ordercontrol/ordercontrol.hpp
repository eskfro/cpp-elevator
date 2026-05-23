#pragma once

#include <common/config.hpp>
#include <common/types.hpp>

namespace elev::ordercontrol {

class OrderTable {
    private:
        elev::common::OrderStatus table[elev::config::N_ELEVS][elev::config::N_FLOORS][elev::config::N_BUTTONS];

    public:
        OrderTable();
        void clearTable();
        void clearAbove(int f);
        void clearBelow(int f);

        elev::common::OrderStatus getStatusAt(int elevID, int floor, int btn);
        void setStatusAt(int elevID, int floor, int btn, elev::common::OrderStatus status);
        

};

}