#include <common/types.hpp>
#include <ordercontrol/ordercontrol.hpp>

namespace elev::ordercontrol {
    
    
OrderTable::OrderTable() {
    clearTable();
}

void OrderTable::clearTable() {
    using namespace elev::config;

    for (int e = 0; e < N_ELEVS; e++) {
        for (int f = 0; f < N_FLOORS; f++) {
            for (int b = 0; b < N_BUTTONS; b++) {
                this->table[e][f][b] = elev::common::OrderStatus::NONE;
            }
        }
    }
}

elev::common::OrderStatus OrderTable::getStatusAt(int elevID, int floor, int btn) {
    return this->table[elevID][floor][btn];
}
    
}
