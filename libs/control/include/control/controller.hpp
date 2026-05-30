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
        Inertia inertia;

    public:
        Controller();

        void updateRequests(elev::ordersync::OrderSlice slice);

        void setInertia(MotorDir dir);
        void executeDecision(elev::elevator::Elevator* elev, DirMovPair pair);
        int tryCloseDoor(elev::elevator::Elevator* elev);

        // Event driven FSM
        ButtonFlags fsm_table_update(elev::elevator::Elevator* elev);
        ButtonFlags fsm_floor_arrival(elev::elevator::Elevator* elev);
        ButtonFlags fsm_door_timeout(elev::elevator::Elevator* elev);
        ButtonFlags fsm_emergency_stop(elev::elevator::Elevator* elev);

        // Change values on table 
        ButtonFlags clearCurrentFloor(int floor);

        // Decisions
        bool shouldStop(int floor);
        bool shouldClearImmediately(int floor, int btnFloor, BtnType btn);
        DirMovPair chooseDirection(int floor);

        // get
        RequestTable getRequests();
        DoorTimer* getDoorTimer();

};






}