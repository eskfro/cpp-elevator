#pragma once

#include <array>

// Libs
#include <elevator/elevator.hpp>
#include <control/requests.hpp>

using namespace elev::common;

namespace elev::control {

class Controller {
    private:
        RequestTable requests{};

    public:
        Controller();
        ~Controller();

        // Event driven state machine
        void fsm_table_update(elev::elevator::Elevator* elev);
        void fsm_floor_arrival(elev::elevator::Elevator* elev, int floor);
        void fsm_door_timeout(elev::elevator::Elevator* elev);

        // Decisions
        ButtonFlags clearCurrentFloor(int floor, MotorDir dir);
        bool shouldStop(int floor, MotorDir dir);
        bool shouldClearImmediately(int floor, MotorDir dir, int btnFloor, BtnType btn);
        DirMovPair chooseDirection(int floor, MotorDir dir);


};






}