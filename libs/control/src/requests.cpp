#include "common/config.hpp"
#include <array>
#include <control/requests.hpp>

namespace elev::control {


bool RequestTable::Equals(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
   
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            if (table_[f][b] != rhs.Value(f, b)) {
                return false;
            }
        }
    }
    return true;

}


RequestTable RequestTable::CopyFrom(RequestTable rhs) {
    using namespace elev::config;
    using namespace elev::common;
    RequestTable res{};
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            res.SetValue(f, b, rhs.Value(f, b));  
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


std::array<std::array<bool, config::N_BUTTONS>, config::N_FLOORS> RequestTable::Table() {
    std::array<std::array<bool, config::N_BUTTONS>, config::N_FLOORS> res{};

    for (int f = 0; f < config::N_FLOORS; f++) {
        for (int b = 0; b < config::N_BUTTONS; b++) {
            res[f][b] = table_[f][b];
        }
    }
    return res;
}


void RequestTable::SetValue(int floor, int btn, bool value) {
    this->table_[floor][btn] = value;
}


bool RequestTable::Value(int floor, int btn) {
    return table_[floor][btn];
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