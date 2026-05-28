#pragma once

#include <array>

// Libs
#include <elevator/elevator.hpp>
#include <control/requests.hpp>
#include <ordersync/ordersync.hpp>
#include <common/timer.hpp>

using namespace elev::common;

namespace elev::control {

class Controller {
    private:
        RequestTable requests;
        DoorTimer doortimer;

    public:
        Controller();

        void updateRequests(elev::ordersync::OrderSlice slice);

        // Event driven FSM
        ButtonFlags fsm_table_update(elev::elevator::Elevator* elev);
        ButtonFlags fsm_floor_arrival(elev::elevator::Elevator* elev);
        ButtonFlags fsm_door_timeout(elev::elevator::Elevator* elev);

        // Change values on table 
        ButtonFlags clearCurrentFloor(int floor, MotorDir dir);

        // Decisions
        bool shouldStop(int floor, MotorDir dir);
        bool shouldClearImmediately(int floor, MotorDir dir, int btnFloor, BtnType btn);
        DirMovPair chooseDirection(int floor, MotorDir dir);

        // get
        RequestTable getRequests();
        DoorTimer* getDoorTimer();

};






}