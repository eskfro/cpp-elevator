#pragma once

#include <common/types.hpp>
#include <common/config.hpp>

namespace elev::movement {

class RequestTable {
    private:
        //Table used by single elevator
        bool table[elev::config::N_FLOORS][elev::config::N_BUTTONS]; 
    
    public:
        RequestTable();

        void setValueAt(int floor, elev::common::BtnType btn, bool value);
        bool getValueAt(int floor, elev::common::BtnType btn);

        void clearCurrentFloor(int floor, elev::common::MotorDir dir);

        bool isRequestAbove(int floor);
        bool isRequestBelow(int floor);
        bool isRequestHere(int floor);

        bool shouldStop(int floor, elev::common::MotorDir dir);
        bool shouldClearImmediately(int floor, elev::common::MotorDir dir, int btnFloor, elev::common::BtnType btn);

        elev::common::DirMovPair chooseDirection(int floor, elev::common::MotorDir dir);

};

}