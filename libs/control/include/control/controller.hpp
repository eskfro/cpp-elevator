#pragma once

#include <array>

// Libs
#include <elevator/elevator.hpp>
#include <control/requests.hpp>

namespace elev::control {


class Controller {
    private:
        elev::elevator::Elevator elev;
        RequestTable requests;

    public:

        // Event driven state machine
        void onTableUpdate(elev::elevator::Elevator* elev);
        void onFloorArrival(elev::elevator::Elevator* elev);
        void onDoorTimeout(elev::elevator::Elevator* elev);

        // Decisions
        std::array<bool, elev::config::N_BUTTONS> clearCurrentFloor(int floor, elev::common::MotorDir dir);
        bool shouldStop(int floor, elev::common::MotorDir dir);
        bool shouldClearImmediately(int floor, elev::common::MotorDir dir, int btnFloor, elev::common::BtnType btn);
        elev::common::DirMovPair chooseDirection(int floor, elev::common::MotorDir dir);


};






}