#include <control/requests.hpp>

namespace elev::control {


bool RequestTable::is_equal(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
   
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (table_[f][b] != rhs.Value(f, (BtnType)b)) {
                return false;
            }
        }
    }
    return true;

}


RequestTable RequestTable::set_equal(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
    RequestTable res{};
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            res.SetValue(f, (BtnType)b, rhs.Value(f, (BtnType)b));  
        }
    }
    return res;

}


RequestTable::RequestTable() {
    using namespace elev::config;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            table_[f][b] = false;
        }
    }
}


void RequestTable::SetValue(int floor, elev::common::BtnType btn, bool value) {
    this->table_[floor][(int)btn] = value;
}


bool RequestTable::Value(int floor, elev::common::BtnType btn) {
    return table_[floor][(int)btn];
}


bool RequestTable::IsRequestHere(int floor) {
    using namespace elev::config;
    for (int b = 0; b < N_BUTTONS; b++) if (this->table_[floor][b]) return true;
    return false;
}


bool RequestTable::IsRequestAbove(int floor) {
    using namespace elev::config;
    for (int f = floor+1; f < N_FLOORS; f++) if (IsRequestHere(f)) return true;
    return false;
}


bool RequestTable::IsRequestBelow(int floor) {
    for (int df = floor-1; df >= 0; df--) if (IsRequestHere(df)) return true;
    return false;
}



}