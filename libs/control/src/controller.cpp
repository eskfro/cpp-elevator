#include <control/controller.hpp>

#include "common/config.hpp"
#include "common/types.hpp"
#include "elevator/elevator.hpp"

namespace elev::control {

Controller::Controller() {
    doortimer_.Stop();
    floor_timer_.Stop();
    inertia_ = Inertia::None;
}

Timer* Controller::DoorTimer() { return &doortimer_; }

Timer* Controller::FloorTimer() { return &floor_timer_; }

RequestTable Controller::Requests() { return requests_; }

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
    ButtonFlags zero{};

    elev->State()->SetStopped(true);
    elev->SetMotorDir(MotorDir::Stop);
    elev->SetStopLamp(1);

    if (elev->FloorSensor() != kBetweenFloors) {
        if (!elev->State()->DoorOpen()) elev->OpenDoor();
        doortimer_.Start(kDoorOpenTimeMs);
    }

    elev->State()->SetMovingState(MovingState::Idle);
    floor_timer_.Stop();

    return zero;
}

// FSM Emergency Stop Reset
ButtonFlags Controller::FsmEmergencyStopReset(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Emergency Stop Reset" << std::endl;
    ButtonFlags zero{};

    elev->State()->SetStopped(false);
    elev->SetStopLamp(0);

    const int floor = elev->State()->Floor();
    if (elev->State()->DoorOpen()) {
        elev->State()->SetMovingState(MovingState::DoorOpen);
        doortimer_.Start(kDoorOpenTimeMs);
        return ClearCurrentFloor(floor);
    }

    // Stopped between floors: ChooseDirection can't reason about a floor
    // relative to kBetweenFloors, so resume in the direction we were
    // already moving instead. Normal floor-arrival logic takes back over
    // once a real floor is reached.
    DirMovPair pair{MotorDir::Stop, MovingState::Idle};
    if (inertia_ == Inertia::Up) pair = {MotorDir::Up, MovingState::Moving};
    if (inertia_ == Inertia::Down) pair = {MotorDir::Down, MovingState::Moving};
    ExecuteDecision(elev, pair);
    return zero;
}

// FSM Table Update
ButtonFlags Controller::FsmTableUpdate(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Table Update" << std::endl;

    using namespace elev::common;

    DirMovPair pair{};
    ButtonFlags zero{};
    int floor = elev->State()->Floor();

    switch (elev->State()->MovingState()) {
        case MovingState::DoorOpen:
            if (ShouldStop(floor)) {
                doortimer_.Start(kDoorOpenTimeMs);
                return ClearCurrentFloor(floor);
            }
            return zero;

        case MovingState::Moving:
            return zero;
        case MovingState::Idle:
            pair = ChooseDirection(floor);
            switch (pair.moving_state) {
                case MovingState::DoorOpen:
                    if (elev->State()->Floor() != kBetweenFloors) {
                        elev->OpenDoor();
                    }
                    doortimer_.Start(kDoorOpenTimeMs);
                    ExecuteDecision(elev, pair);
                    return ClearCurrentFloor(floor);
                case MovingState::Moving:
                case MovingState::Idle:
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
ButtonFlags Controller::FsmFloorArrival(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Arrived @ Floor "
              << elev->State()->Floor() << std::endl;
    using namespace elev::common;

    ButtonFlags zero{};
    int floor = elev->State()->Floor();

    floor_timer_.Start(kFaultTimeoutMs);
    elev->State()->SetFault(false);

    elev->SetFloorIndicator();

    switch (elev->State()->MovingState()) {
        case MovingState::Moving:
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
ButtonFlags Controller::FsmDoorTimeout(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator " << elev->State()->Id() << " ] - FSM: Door Timeout" << std::endl;
    using namespace elev::common;

    doortimer_.Stop();

    DirMovPair pair;
    ButtonFlags zero{};
    int floor = elev->State()->Floor();

    elev->State()->SetObstruction(elev->ObstructionSignal());

    if (elev->State()->Obstruction()) {
        common::PrintError("[FSM] Obs!");
        doortimer_.Start(kDoorOpenTimeMs);
        return zero;
    }
    switch (elev->State()->MovingState()) {
        case MovingState::DoorOpen:
            pair = ChooseDirection(floor);
            switch (pair.moving_state) {
                case MovingState::DoorOpen:
                    ExecuteDecision(elev, pair);
                    doortimer_.Start(kDoorOpenTimeMs);
                    return ClearCurrentFloor(elev->State()->Floor());
                case MovingState::Moving:
                case MovingState::Idle:
                case MovingState::Err:  // TODO
                    if (TryCloseDoor(elev)) {
                        ExecuteDecision(elev, pair);
                    }
                    return zero;
            }

        default:
            return zero;
    }
}

// FSM Floor Timeout
ButtonFlags Controller::FsmFloorTimeout(elev::elevator::Elevator* elev) {
    common::PrintError("[FSM] Floor timeout - fault");
    using namespace elev::common;

    floor_timer_.Stop();

    if (elev->State()->MovingState() == MovingState::Moving) {
        elev->State()->SetFault(true);
    }

    return ButtonFlags{};
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

elev::common::DirMovPair Controller::ChooseDirection(int floor) {
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

bool Controller::ShouldStop(int floor) {
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

bool Controller::ShouldClearImmediately(int floor, int btn_floor, elev::common::BtnType btn) {
    using namespace elev::common;
    return floor == btn_floor &&
           ((inertia_ == Inertia::Up && btn == BtnType::HallUp) ||
            (inertia_ == Inertia::Down && btn == BtnType::HallDown) ||
            (btn == BtnType::Cab));
}

ButtonFlags Controller::ClearCurrentFloor(int floor) {
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
