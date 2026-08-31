#include <control/controller.hpp>

#include "common/config.hpp"
#include "common/types.hpp"
#include "elevator/elevator.hpp"

namespace {

constexpr ButtonFlags kNoClear = ButtonFlags{};

} // namespace

namespace elev::control {

Controller::Controller() {
    doortimer_.Stop();
    floor_timer_.Stop();
    inertia_ = Inertia::None;
}

void Controller::Init() {
    floor_timer_.Stop();
}

Timer* Controller::DoorTimer() { return &doortimer_; }

Timer* Controller::FloorTimer() { return &floor_timer_; }

RequestTable Controller::Requests() const { return requests_; }

void Controller::SetInertia(elev::elevator::Elevator* elev, MotorDir dir) {
    if (dir == MotorDir::Up) inertia_ = Inertia::Up;
    if (dir == MotorDir::Down) inertia_ = Inertia::Down;

    elev->State()->SetInertia(inertia_);
}

void Controller::SetRequests(BoolTable bool_table) {
    using namespace elev::common;
    // yup
    for (int f = 0; f < kFloors; f++) for (int b = 0; b < kButtons; b++) requests_.SetValue(f, b, bool_table[f][b]);
}

// FSM Emergency Stop
ButtonFlags Controller::FsmEmergencyStop(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Emergency Stop" << std::endl;

    elev->State()->SetStopped(true);
    elev->SetMotorDir(MotorDir::Stop);
    elev->SetStopLamp(1);

    if (elev->FloorSensor() != kBetweenFloors) {
        if (!elev->State()->DoorOpen()) elev->OpenDoor();
        doortimer_.Start(kDoorOpenTimeMs);
    }

    elev->State()->SetMovingState(MovingState::Idle);
    floor_timer_.Stop();

    return kNoClear;
}

// FSM Emergency Stop Reset
ButtonFlags Controller::FsmEmergencyStopReset(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Emergency Stop Reset" << std::endl;
    const int floor = elev->State()->Floor();

    elev->State()->SetStopped(false);
    elev->SetStopLamp(0);

    // Reset at DoorOpen
    if (elev->State()->DoorOpen()) {
        elev->State()->SetMovingState(MovingState::DoorOpen);
        doortimer_.Start(kDoorOpenTimeMs);
        return ClearCurrentFloor(floor);
    }

    // Set new direction after reset
    DirMovPair pair{MotorDir::Stop, MovingState::Idle};
    if (inertia_ == Inertia::Up) pair = {MotorDir::Up, MovingState::Moving};
    if (inertia_ == Inertia::Down) pair = {MotorDir::Down, MovingState::Moving};
    ExecuteDecision(elev, pair);
    return kNoClear;
}

ButtonFlags Controller::FsmTableUpdate(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Table Update" << std::endl;
    using namespace elev::common;
    const int floor = elev->State()->Floor();
    const MovingState mov = elev->State()->MovingState();

    if (mov == MovingState::DoorOpen) {
        if (!ShouldStop(floor)) return kNoClear;
        doortimer_.Start(kDoorOpenTimeMs);
        return ClearCurrentFloor(floor);
    }

    // Exit while moving or err
    if (mov == common::MovingState::Moving || 
        mov == common::MovingState::Err) {
        return kNoClear;
    }

    // Idle
    DirMovPair pair = ChooseDirection(floor);
    if (pair.moving_state == MovingState::DoorOpen) {
        if (elev->State()->Floor() != kBetweenFloors) elev->OpenDoor();
        doortimer_.Start(kDoorOpenTimeMs);
        ExecuteDecision(elev, pair);
        return ClearCurrentFloor(floor);
    }

    // Move in new direction
    if (TryCloseDoor(elev)) ExecuteDecision(elev, pair);

    return kNoClear;
}

ButtonFlags Controller::FsmFloorArrival(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Arrived @ Floor " << elev->State()->Floor() << std::endl;
    using namespace elev::common;
    const int floor = elev->State()->Floor();

    elev->SetFloorIndicator();
    elev->State()->SetFault(false);
    floor_timer_.Start(kFaultTimeoutMs);

    if (elev->State()->MovingState() != MovingState::Moving) return kNoClear;
    if (!ShouldStop(floor)) return kNoClear;

    StopAndOpenDoor(elev);
    return ClearCurrentFloor(floor);
}

ButtonFlags Controller::FsmDoorTimeout(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Door Timeout" << std::endl;
    using namespace elev::common;
    const int floor = elev->State()->Floor();
    
    doortimer_.Stop();

    if (elev->State()->Obstruction()) {
        common::PrintError("[FSM] Obs!");
        doortimer_.Start(kDoorOpenTimeMs);
        return kNoClear;
    }

    if (elev->State()->MovingState() != MovingState::DoorOpen) return kNoClear;

    DirMovPair pair = ChooseDirection(floor);
    if (pair.moving_state == MovingState::DoorOpen) {
        ExecuteDecision(elev, pair);
        doortimer_.Start(kDoorOpenTimeMs);
        return ClearCurrentFloor(elev->State()->Floor());
    }

    if (TryCloseDoor(elev)) ExecuteDecision(elev, pair);

    return kNoClear;
}

// FSM Floor Timeout
ButtonFlags Controller::FsmFloorTimeout(elev::elevator::Elevator* elev) {
    common::PrintError("[FSM] Floor timeout - fault");
    using namespace elev::common;

    floor_timer_.Stop();

    if (elev->State()->MovingState() == MovingState::Moving) {
        elev->State()->SetFault(true);
    }

    return kNoClear;
}

void Controller::StopAndOpenDoor(elev::elevator::Elevator* elev) {
    elev->SetMotorDir(MotorDir::Stop);
    elev->OpenDoor();
    doortimer_.Start(kDoorOpenTimeMs);
    floor_timer_.Stop();
}

void Controller::ExecuteDecision(elev::elevator::Elevator* elev, DirMovPair pair) {
    if (elev->State()->Stopped()) return;

    elev->SetMotorDir(pair.motor_dir);
    elev->State()->SetMovingState(pair.moving_state);

    if (pair.moving_state == MovingState::Moving) {
        floor_timer_.Start(kFaultTimeoutMs);
    } else {
        floor_timer_.Stop();
    }

    SetInertia(elev, pair.motor_dir);
}

int Controller::TryCloseDoor(elev::elevator::Elevator* elev) {
    if (elev->State()->Obstruction()) {
        if (doortimer_.Expired()) doortimer_.Start(kDoorOpenTimeMs);
        return 0;
    } else {
        elev->CloseDoor();
        return 1;
    }
}

elev::common::DirMovPair Controller::ChooseDirection(int floor) const {
    using namespace elev::common;
    switch (inertia_) {
        case Inertia::Up:
            if (requests_.IsRequestAbove(floor)) return {MotorDir::Up, MovingState::Moving};
            if (requests_.IsRequestHere(floor)) return {MotorDir::Stop, MovingState::DoorOpen};
            if (requests_.IsRequestBelow(floor)) return {MotorDir::Down, MovingState::Moving};
            else return {MotorDir::Stop, MovingState::Idle};

        case Inertia::Down:
            if (requests_.IsRequestBelow(floor)) return {MotorDir::Down, MovingState::Moving};
            if (requests_.IsRequestHere(floor)) return {MotorDir::Stop, MovingState::DoorOpen};
            if (requests_.IsRequestAbove(floor)) return {MotorDir::Up, MovingState::Moving};
            else return {MotorDir::Stop, MovingState::Idle};

        case Inertia::None:
            if (requests_.IsRequestHere(floor)) return {MotorDir::Stop, MovingState::DoorOpen};
            if (requests_.IsRequestAbove(floor)) return {MotorDir::Up, MovingState::Moving};
            if (requests_.IsRequestBelow(floor)) return {MotorDir::Down, MovingState::Moving};
            else return {MotorDir::Stop, MovingState::Idle};

        default: return {MotorDir::Stop, MovingState::Idle};
    }
};

bool Controller::ShouldStop(int floor) const {
    using namespace elev::common;
    switch (inertia_) {
        case (Inertia::Down):
            return requests_.Value(floor, (int)BtnType::HallDown) ||
                   requests_.Value(floor, (int)BtnType::Cab) ||
                   !requests_.IsRequestBelow(floor);
        case (Inertia::Up):
            return requests_.Value(floor, (int)BtnType::HallUp) ||
                   requests_.Value(floor, (int)BtnType::Cab) ||
                   !requests_.IsRequestAbove(floor);
        case (Inertia::None):
        default:
            return true;
    }
}

bool Controller::ShouldClearImmediately(int floor, int btn_floor, elev::common::BtnType btn) const {
    using namespace elev::common;
    return floor == btn_floor &&
           ((inertia_ == Inertia::Up && btn == BtnType::HallUp) ||
            (inertia_ == Inertia::Down && btn == BtnType::HallDown) ||
            (btn == BtnType::Cab));
}

ButtonFlags Controller::ClearCurrentFloor(int floor) const {
    using namespace elev::common;

    // Buttons to clear
    ButtonFlags b2c{};

    // Always clear cab
    b2c[(int)BtnType::Cab] = true;

    // Clearing hall calls
    switch (inertia_) {
        case Inertia::Up:
            if (!requests_.IsRequestAbove(floor) &&
                !requests_.Value(floor, (int)BtnType::HallUp)) {
                b2c[(int)BtnType::HallDown] = true;
            }
            b2c[(int)BtnType::HallUp] = true;
            return b2c;

        case Inertia::Down:
            if (!requests_.IsRequestBelow(floor) &&
                !requests_.Value(floor, (int)BtnType::HallDown)) {
                b2c[(int)BtnType::HallUp] = true;
            }
            b2c[(int)BtnType::HallDown] = true;
            return b2c;

        default:
            b2c[(int)BtnType::HallUp] = true;
            b2c[(int)BtnType::HallDown] = true;
            return b2c;
    }
}

bool Controller::RequestTableUpdated() {
    bool res = false;
    if (requests_.Table() != prev_requests_.Table()) {
        res = true;
    }
    prev_requests_ = requests_;
    return res;
}

}  // namespace elev::control
