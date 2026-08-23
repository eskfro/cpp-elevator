#pragma once

#include <common/config.hpp>
#include <common/types.hpp>

namespace elev::control {

class RequestTable {
public:
    RequestTable() = default;

    // Set
    void SetValue(int floor, int btn, bool value);

    // Get
    bool Value(int floor, int btn);
    common::BoolTable Table();

    bool IsRequestAbove(int floor);
    bool IsRequestBelow(int floor);
    bool IsRequestHere(int floor);

private:
    common::BoolTable table_{};
};

}  // namespace elev::control