#pragma once

#include <common/types.hpp>
#include <common/config.hpp>

namespace elev::control {

class RequestTable {
    private:
        //Table used by single elevator
        bool table_[elev::config::N_FLOORS][elev::config::N_BUTTONS]; 
    
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

};

} // namespace elev::control