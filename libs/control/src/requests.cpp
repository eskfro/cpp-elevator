#include <array>
#include <control/requests.hpp>

#include "common/config.hpp"

namespace elev::control {

std::array<std::array<bool, config::kButtons>, config::kFloors>
RequestTable::Table() {
    return table_;
}

void RequestTable::SetValue(int floor, int btn, bool value) {
    if (floor < 0 || floor >= config::kFloors) return;
    if (btn < 0 || btn >= config::kButtons) return;

    this->table_[floor][btn] = value;
}

bool RequestTable::Value(int floor, int btn) {
    if (floor < 0 || floor >= config::kFloors) return false;

    return table_[floor][btn];
}

bool RequestTable::IsRequestHere(int floor) {
    using namespace elev::config;
    if (floor < 0 || floor >= kFloors) return false;

    for (int b = 0; b < kButtons; b++)
        if (this->table_[floor][b]) return true;
    return false;
}

bool RequestTable::IsRequestAbove(int floor) {
    using namespace elev::config;
    for (int f = floor + 1; f < kFloors; f++)
        if (IsRequestHere(f)) return true;
    return false;
}

bool RequestTable::IsRequestBelow(int floor) {
    for (int df = floor - 1; df >= 0; df--)
        if (IsRequestHere(df)) return true;
    return false;
}

}  // namespace elev::control