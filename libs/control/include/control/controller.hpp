#pragma once

#include <array>

// Libs
#include <elevator/elevator.hpp>
#include <control/requests.hpp>

using namespace elev::common;

namespace elev::control {

class Controller {
    private:
        RequestTable requests;

    public:
        Controller();
        ~Controller();

        // Event driven state machine
        void onTableUpdate(elev::elevator::Elevator* elev);
        void onFloorArrival(elev::elevator::Elevator* elev);
        void onDoorTimeout(elev::elevator::Elevator* elev);

        // Decisions
        ButtonFlags clearCurrentFloor(int floor, MotorDir dir);
        bool shouldStop(int floor, MotorDir dir);
        bool shouldClearImmediately(int floor, MotorDir dir, int btnFloor, BtnType btn);
        DirMovPair chooseDirection(int floor, MotorDir dir);


};






}