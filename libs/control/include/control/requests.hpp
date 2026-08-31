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
    bool Value(int floor, int btn) const;
    common::BoolTable Table() const;

    bool IsRequestAbove(int floor) const;
    bool IsRequestBelow(int floor) const;
    bool IsRequestHere(int floor) const;

private:
    common::BoolTable table_{};
};

}  // namespace elev::control