#include <control/requests.hpp>

namespace elev::control {


bool RequestTable::equal(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
   
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (table[f][b] != rhs.getValueAt(f, (BtnType)b)) {
                return false;
            }
        }
    }
    return true;

}


RequestTable RequestTable::copy(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
    RequestTable res{};
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            res.setValueAt(f, (BtnType)b, rhs.getValueAt(f, (BtnType)b));  
        }
    }
    return res;

}


RequestTable::RequestTable() {
    using namespace elev::config;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            table[f][b] = false;
        }
    }
}


void RequestTable::setValueAt(int floor, elev::common::BtnType btn, bool value) {
    this->table[floor][(int)btn] = value;
}


bool RequestTable::getValueAt(int floor, elev::common::BtnType btn) {
    return table[floor][(int)btn];
}


bool RequestTable::isRequestHere(int floor) {
    using namespace elev::config;
    for (int b = 0; b < N_BUTTONS; b++) if (this->table[floor][b]) return true;
    return false;
}


bool RequestTable::isRequestAbove(int floor) {
    using namespace elev::config;
    for (int f = floor+1; f < N_FLOORS; f++) if (isRequestHere(f)) return true;
    return false;
}


bool RequestTable::isRequestBelow(int floor) {
    for (int df = floor-1; df >= 0; df--) if (isRequestHere(df)) return true;
    return false;
}



}