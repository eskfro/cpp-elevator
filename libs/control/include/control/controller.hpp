#pragma once

// Libs
#include <common/timer.hpp>
#include <control/requests.hpp>
#include <elevator/elevator.hpp>
#include <ordersync/ordersync.hpp>

using namespace elev::common;

namespace elev::control {

class Controller {
public:
    Controller();

    // Set
    void SetRequests(BoolTable bool_table);
    void SetInertia(elev::elevator::Elevator* elev, MotorDir dir);

    void ExecuteDecision(elev::elevator::Elevator* elev, DirMovPair pair);
    void StopAndOpenDoor(elev::elevator::Elevator* elev);
    int TryCloseDoor(elev::elevator::Elevator* elev);

    // Event driven FSM
    ButtonFlags FsmTableUpdate(elev::elevator::Elevator* elev);
    ButtonFlags FsmFloorArrival(elev::elevator::Elevator* elev);
    ButtonFlags FsmDoorTimeout(elev::elevator::Elevator* elev);
    ButtonFlags FsmEmergencyStop(elev::elevator::Elevator* elev);
    ButtonFlags FsmFloorTimeout(elev::elevator::Elevator* elev);

    // Change values on table
    ButtonFlags ClearCurrentFloor(int floor);

    // Decisions
    bool ShouldStop(int floor);
    bool ShouldClearImmediately(int floor, int btn_floor, BtnType btn);
    bool RequestTableUpdated();
    DirMovPair ChooseDirection(int floor);

    // Get
    RequestTable Requests();
    common::Timer* DoorTimer();
    common::Timer* FloorTimer();

private:
    RequestTable prev_requests_{};
    RequestTable requests_{};
    common::Timer doortimer_;
    common::Timer floor_timer_;
    Inertia inertia_;
};

}  // namespace elev::control