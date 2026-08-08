#pragma once

#include <array>
#include <common/types.hpp>
#include <common/config.hpp>

namespace elev::control {

class RequestTable {
    public:
        RequestTable() = default;

        // Set
        void SetValue(int floor, int btn, bool value);

        // Get
        bool Value(int floor, int btn);
        std::array<std::array<bool, config::kButtons>, config::kFloors> Table();

        bool IsRequestAbove(int floor);
        bool IsRequestBelow(int floor);
        bool IsRequestHere(int floor);

    
    private:
        std::array<std::array<bool, config::kButtons>, config::kFloors> table_{};

};

} // namespace elev::control