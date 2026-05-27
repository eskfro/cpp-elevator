#pragma once

#include <common/types.hpp>
#include <common/config.hpp>

namespace elev::control {

class RequestTable {
    private:
        //Table used by single elevator
        bool table[elev::config::N_FLOORS][elev::config::N_BUTTONS]; 
    
    public:
        RequestTable();

        void setValueAt(int floor, elev::common::BtnType btn, bool value);
        bool getValueAt(int floor, elev::common::BtnType btn);

        bool isRequestAbove(int floor);
        bool isRequestBelow(int floor);
        bool isRequestHere(int floor);

        // Operator overloading lol
        RequestTable copy(RequestTable rhs);
        bool equal(RequestTable rhs);

};

}