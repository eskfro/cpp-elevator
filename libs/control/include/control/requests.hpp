#pragma once

#include <array>
#include <common/types.hpp>
#include <common/config.hpp>

namespace elev::control {

class RequestTable {
    public:
        RequestTable();

        void SetValue(int floor, int btn, bool value);
        bool Value(int floor, int btn);

        bool IsRequestAbove(int floor);
        bool IsRequestBelow(int floor);
        bool IsRequestHere(int floor);

        // Operator overloading lol
        RequestTable CopyFrom(RequestTable rhs);
        bool Equals(RequestTable rhs);

        std::array<std::array<bool, config::kButtons>, config::kFloors> Table();
    
    private:
        std::array<std::array<bool, config::kButtons>, config::kFloors> table_{};

};

} // namespace elev::control