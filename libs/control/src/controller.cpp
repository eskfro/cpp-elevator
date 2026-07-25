#include "common/config.hpp"
#include <control/controller.hpp>

namespace elev::control {


Controller::Controller() {
    doortimer_.Stop();
    inertia_ = Inertia::NONE;
}


DoorTimer* Controller::Doortimer() {
    return &doortimer_;
}


void Controller::SetInertia(MotorDir dir) {
    if (dir == MotorDir::UP) inertia_ = Inertia::UP;
    if (dir == MotorDir::DOWN) inertia_ = Inertia::DOWN;
}


RequestTable Controller::Requests() {
    return this->requests_;
}


bool Controller::IsRequestsChanged(elev::control::RequestTable prev_requests) {
    if (requests_.is_equal(prev_requests)) return false;
    else return true;
}


void Controller::UpdateRequests(std::array<std::array<bool, N_BUTTONS>, N_FLOORS> bool_table) {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            requests_.SetValue(f, (BtnType)b, bool_table[f][b]);
        }
    }
}


// FSM Emergency Stop
ButtonFlags Controller::_fsm_emergency_stop(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->ID() << " ] - FSM: Emergency Stop" << std::endl;
    ButtonFlags zero{};

    elev->SetStop(true);
    elev->SetMotorDir(MotorDir::STOP);
    elev->SetStopLamp(1);

    if (elev->FloorSensor() != BETWEEN_FLOORS) {
        if (!elev->DoorOpen()) elev->OpenDoor();
        doortimer_.Start(DOOR_OPEN_TIME_MS);
    }

    inertia_ = Inertia::NONE;
    elev->SetMovingState(MovingState::IDLE);
    return zero;
}


// FSM Table Update
ButtonFlags Controller::_fsm_table_update(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->ID() << " ] - FSM: Table Update" << std::endl;

    using namespace elev::common;

    DirMovPair pair{};
    ButtonFlags zero{};
    int floor = elev->Floor();

    switch (elev->MovingState()) {
        case MovingState::DOOR_OPEN:
            if (ShouldStop(floor)) {
                doortimer_.Start(DOOR_OPEN_TIME_MS);
                return ClearCurrentFloor(floor);
            }
            return zero;
            
        case MovingState::MOVING:
            return zero;
        case MovingState::IDLE:
            pair = ChooseDirection(floor);
            switch (elev->MovingState()) {
                case MovingState::DOOR_OPEN:
                    if (elev->Floor() != BETWEEN_FLOORS) {
                        elev->OpenDoor();
                    }
                    doortimer_.Start(DOOR_OPEN_TIME_MS);
                    ExecuteDecision(elev, pair);
                    return ClearCurrentFloor(floor);
                case MovingState::MOVING:
                case MovingState::IDLE:
                    if (TryCloseDoor(elev)) {
                        ExecuteDecision(elev, pair);
                    }
                    return zero;

                default:
                    return zero;
            }
        default:
            return zero;        
    }
}

// FSM Floor Arrival
ButtonFlags Controller::_fsm_floor_arrival(elev::elevator::Elevator* elev) {
     std::cout << "[ Elevator "<< elev->ID() << " ] - FSM: Arrived @ Floor " << elev->Floor() << std::endl; 
    using namespace elev::common;

    ButtonFlags zero{};
    int floor = elev->Floor();
    elev->SetFloorIndicator();

    switch (elev->MovingState()) {
        case MovingState::MOVING:
            if (ShouldStop(floor)) {
                StopAndOpenDoor(elev);
                return ClearCurrentFloor(floor);
            }
            return zero;
        default:
            return zero;
    }
}




// FSM Door Timeout
ButtonFlags Controller::_fsm_door_timeout(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->ID() << " ] - FSM: Door Timeout" << std::endl;
    using namespace elev::common;
    
    DirMovPair pair;
    elev->SetObs(elev->ObsSignal());
    ButtonFlags zero{};
    int floor = elev->Floor();

    if (elev->Obs()) {
        std::cout << "OBS!" << std::endl;
        doortimer_.Start(DOOR_OPEN_TIME_MS);
        return zero;
    }
    switch(elev->MovingState()) {
        case MovingState::DOOR_OPEN:
            pair =  ChooseDirection(floor);
            switch (pair.moving_state) {
                case MovingState::DOOR_OPEN:
                    ExecuteDecision(elev, pair);
                    doortimer_.Start(DOOR_OPEN_TIME_MS);
                    return ClearCurrentFloor(elev->Floor());
                case MovingState::MOVING:
                case MovingState::IDLE:
                case MovingState::ERR: // TODO
                    if (TryCloseDoor(elev)) {
                        ExecuteDecision(elev, pair);
                    }
                    return zero;
            }

        default:
            return zero;
    }
}


void Controller::StopAndOpenDoor(elev::elevator::Elevator* elev) {
    elev->SetMotorDir(MotorDir::STOP);
    elev->OpenDoor();
    doortimer_.Start(DOOR_OPEN_TIME_MS);
}


void Controller::ExecuteDecision(elev::elevator::Elevator* elev, DirMovPair pair) {

    if (elev->Stop()) return;

    elev->SetMotorDir(pair.motor_dir);
    elev->SetMovingState(pair.moving_state);
    SetInertia(pair.motor_dir);       
}


int Controller::TryCloseDoor(elev::elevator::Elevator* elev) {
    if (elev->Obs()) {
        if (doortimer_.Expired()) doortimer_.Start(DOOR_OPEN_TIME_MS);
        return 0;
    } else {
        elev->CloseDoor();
        return 1;
    }
}   


elev::common::DirMovPair Controller::ChooseDirection(int floor) {
    using namespace elev::common;
    switch (inertia_) {
        case Inertia::UP:
        if (requests_.IsRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
        if (requests_.IsRequestHere(floor))  return {MotorDir::DOWN, MovingState::DOOR_OPEN};
        if (requests_.IsRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
        else return {MotorDir::STOP, MovingState::IDLE};
        
        case Inertia::DOWN:
        if (requests_.IsRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
            if (requests_.IsRequestHere(floor))  return {MotorDir::UP,   MovingState::DOOR_OPEN};
            if (requests_.IsRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
            else return {MotorDir::STOP, MovingState::IDLE};
            
        case Inertia::NONE:
            if (requests_.IsRequestHere(floor))  return {MotorDir::STOP, MovingState::DOOR_OPEN};
            if (requests_.IsRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
            if (requests_.IsRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
            else return {MotorDir::STOP, MovingState::IDLE};

        default:
            return {MotorDir::STOP, MovingState::IDLE};
    }
};


bool Controller::ShouldStop(int floor) {
    using namespace elev::common;
    switch (inertia_) {
    case(Inertia::DOWN):
        return requests_.Value(floor, BtnType::HALL_DOWN) || requests_.Value(floor, BtnType::CAB) || !requests_.IsRequestBelow(floor);
    case(Inertia::UP):
        return requests_.Value(floor, BtnType::HALL_UP) || requests_.Value(floor, BtnType::CAB) || !requests_.IsRequestAbove(floor);
    case(Inertia::NONE):
    default:
        return true;
    }
}


bool Controller::ShouldClearImmediately(int floor, int btnFloor, elev::common::BtnType btn) {
    using namespace elev::common;
    return floor == btnFloor &&
    (
        (inertia_ == Inertia::UP   && btn == BtnType::HALL_UP)   || 
        (inertia_ == Inertia::DOWN && btn == BtnType::HALL_DOWN) ||
        (btn == BtnType::CAB) 

    );
}


ButtonFlags Controller::ClearCurrentFloor(int floor) {
    using namespace elev::common;

    // Buttons to clear
    ButtonFlags b2c{};

    // Always clear cab
    b2c[(int)BtnType::CAB] = true;

    // Clearing hall calls
    switch(inertia_) {
        case Inertia::UP:
            if (!requests_.IsRequestAbove(floor) && !requests_.Value(floor, BtnType::HALL_UP)) {
                b2c[(int)BtnType::HALL_DOWN] = true;
            }
            b2c[(int)BtnType::HALL_UP] = true;
            return b2c;

        case Inertia::DOWN:
            if (!requests_.IsRequestBelow(floor) && !requests_.Value(floor, BtnType::HALL_DOWN)) {
                b2c[(int)BtnType::HALL_UP] = true;
            }
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

        default:
            b2c[(int)BtnType::HALL_UP] = true;
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

    }



}

};

