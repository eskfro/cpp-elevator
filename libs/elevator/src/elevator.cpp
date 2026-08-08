#include "buttons/button.hpp"
#include "common/types.hpp"
#include <mutex>
#include <thread>
#include <chrono> 

// Libs
#include <elevator/elevator.hpp>
#include <hardware/hardware.hpp>

namespace elev::elevator {


Elevator::Elevator(int id, std::string ip) {
    buttons_ = elev::buttons::ButtonMatrix();
    state_.SetID(id);
    state_.SetIP(ip);
    state_.SetFloor(FloorSensor());
    state_.SetStopped(false);
    state_.SetMotorDir(common::MotorDir::STOP);
    state_.SetMovingState(common::MovingState::IDLE);
    state_.SetObstruction(ObstructionSignal());
    state_.SetActivity(true);
    state_.SetDoorOpen(false);
}


void Elevator::InitToFloor() {
    SetMotorDir(elev::common::MotorDir::DOWN);
    while (FloorSensor() == BETWEEN_FLOORS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    SetMotorDir(elev::common::MotorDir::STOP);
}


void Elevator::Step() {
    state_.SetFloor(FloorSensor());
    state_.SetObstruction(ObstructionSignal());
}


void Elevator::Init() {
    InitToFloor();
    state_.Init();
    state_.SetMotorDir(elev::common::MotorDir::STOP);
    state_.SetFloor(FloorSensor());
    state_.SetPrevFloor(FloorSensor());
    state_.SetStopped(StopSignal());
    SetFloorIndicator();
    std::cout << "[ Elevator " << state_.ID() << " ] - INIT to floor " << state_.Floor() << std::endl;
}


ElevatorState Elevator::StateCopy() {
    return state_;
}


bool Elevator::HitNewFloor() {
    bool res = false;
    if (state_.Floor() != state_.PrevFloor() && state_.Floor() != BETWEEN_FLOORS) {
        res = true;
    }
    state_.SetPrevFloor(state_.Floor());
    return res;
}


ElevatorState* Elevator::State() {
    return &state_;
}


elev::buttons::ButtonMatrix* Elevator::Buttons() {
    return &buttons_;
}


void Elevator::SetMotorDir(elev::common::MotorDir dir) {
    using namespace elev::common;
    enum MotorDir new_dir;

    if (state_.Stopped()) {
        new_dir = MotorDir::STOP;
    } else {
        new_dir = dir;
    }

    elev::hardware::set_motor_dir(new_dir);
    state_.SetMotorDir(new_dir);
};



void Elevator::SetDoorOpenLamp(int value) {
    elev::hardware::set_door_open_lamp(value);
}


void Elevator::OpenDoor() {
    SetDoorOpenLamp(1);
    state_.SetDoorOpen(true);
    state_.SetMovingState(elev::common::MovingState::DOOR_OPEN);
}


void Elevator::CloseDoor() {
    SetDoorOpenLamp(0);
    state_.SetDoorOpen(false);
}


void Elevator::SetStopLamp(int value) {
    elev::hardware::set_stop_lamp(value);
}   


void Elevator::SetButtonLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::set_btn_lamp(btn, floor, value);
}


void Elevator::SetFloorIndicator() {
    if (state_.Floor() == BETWEEN_FLOORS) return;
    elev::hardware::set_floor_indicator(state_.Floor());
}


int Elevator::FloorSensor() {
    return elev::hardware::get_floor_sensor();
}


int Elevator::StopSignal() {
    return elev::hardware::get_stop_signal();
}


int Elevator::ObstructionSignal() {
    return elev::hardware::get_obstruction_signal();
}


} // namespace elev::elevator
