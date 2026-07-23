#pragma once

// Libs
#include "common/config.hpp"
#include <array>
#include <elevator/elevator.hpp>
#include <control/requests.hpp>
#include <ordersync/ordersync.hpp>
#include <common/timer.hpp>

using namespace elev::common;

namespace elev::control {

class Controller {
    private:
        RequestTable requests_;
        DoorTimer doortimer_;
        Inertia inertia_;

    public:
        Controller();

        void UpdateRequests(std::array<std::array<bool, N_BUTTONS>, N_FLOORS> bool_table);

        void SetInertia(MotorDir dir);
        void ExecuteDecision(elev::elevator::Elevator* elev, DirMovPair pair);
        void StopAndOpenDoor(elev::elevator::Elevator* elev);
        int TryCloseDoor(elev::elevator::Elevator* elev);

        // Event driven FSM
        ButtonFlags _fsm_table_update(elev::elevator::Elevator* elev);
        ButtonFlags _fsm_floor_arrival(elev::elevator::Elevator* elev);
        ButtonFlags _fsm_door_timeout(elev::elevator::Elevator* elev);
        ButtonFlags _fsm_emergency_stop(elev::elevator::Elevator* elev);

        // Change values on table 
        ButtonFlags ClearCurrentFloor(int floor);

        // Decisions
        bool ShouldStop(int floor);
        bool ShouldClearImmediately(int floor, int btnFloor, BtnType btn);
        bool IsRequestsChanged(elev::control::RequestTable prev_requests);

        DirMovPair ChooseDirection(int floor);

        // get
        RequestTable Requests();
        DoorTimer* Doortimer();

};

} // namespace elev::control