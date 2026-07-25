#include "buttons/button.hpp"
#include <thread>
#include <chrono> 

// Libs
#include <elevator/elevator.hpp>
#include <hardware/hardware.hpp>

namespace elev::elevator {


Elevator::Elevator() {
    state_.active = true;

    buttons_ = buttons::ButtonMatrix();

}


bool Elevator::Obs() {
    return state_.obstruction;
}


void Elevator::InitToFloor() {
    SetMotorDir(elev::common::MotorDir::DOWN);

    while (FloorSensor() == BETWEEN_FLOORS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    SetMotorDir(elev::common::MotorDir::STOP);
    SetFloor(FloorSensor());
    SetFloorIndicator();

    std::cout << "[ Elevator " << state_.ID << " ] - INIT to floor " << state_.floor << std::endl;
}


elev::buttons::ButtonMatrix* Elevator::Buttons() {
    return &buttons_;
}


void Elevator::SetIP(std::string _IP) {
    state_.IP = _IP;
}


void Elevator::SetID(int _ID) {
    state_.ID = _ID;
}


void Elevator::SetObs(bool obstruction) {
    this->state_.obstruction = obstruction;
}


bool Elevator::DoorOpen() {
    return this->state_.door_open;
}


void Elevator::SetDoorOpen(bool door_open) {
    this->state_.door_open = door_open;
}


void Elevator::SetFloor(int floor) {
    state_.floor = floor;
}


Elevator::Elevator(int _ID, std::string _IP) {
    state_.ID = _ID;
    state_.IP = _IP;
    state_.active = true;
}


// TODO: idk
void Elevator::SetActivity(bool active) {
    state_.active = active;
};


int Elevator::ID() {
    return state_.ID;
}


std::string Elevator::IP() {
    return state_.IP;
}


void Elevator::SetMotorDir(elev::common::MotorDir dir) {
    using namespace elev::common;
    enum MotorDir newDir;

    if (state_.stop) {
        newDir = MotorDir::STOP;
    } else {
        newDir = dir;
    }

    elev::hardware::set_motor_dir(newDir);
    state_.motor_dir_ = newDir;
};


void Elevator::SetMovingState(elev::common::MovingState mov) {
    state_.moving_state_ = mov;
}


void Elevator::SetDoorOpenLamp(int value) {
    elev::hardware::set_door_open_lamp(value);
}


void Elevator::OpenDoor() {
    this->SetDoorOpenLamp(1);
    this->state_.door_open = true;
    this->SetMovingState(elev::common::MovingState::DOOR_OPEN);
}


void Elevator::CloseDoor() {
    this->SetDoorOpenLamp(0);
    this->state_.door_open = false;
}


void Elevator::SetStopLamp(int value) {
    elev::hardware::set_stop_lamp(value);
}   


void Elevator::SetBtnLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::set_btn_lamp(btn, floor, value);
}


void Elevator::SetFloorIndicator() {
    if (state_.floor == BETWEEN_FLOORS) return;
    elev::hardware::set_floor_indicator(state_.floor);
}


int Elevator::FloorSensor() {
    return elev::hardware::get_floor_sensor();
}


int Elevator::StopSignal() {
    return elev::hardware::get_stop_signal();
}


int Elevator::ObsSignal() {
    return elev::hardware::get_obs_signal();
}


elev::common::MovingState Elevator::MovingState() {
    return state_.moving_state_;
}


int Elevator::Floor() {
    return state_.floor;
}


elev::common::MotorDir Elevator::MotorDir() {
    return state_.motor_dir_;
}


void Elevator::SetStop(bool stop) {
    state_.stop = stop;
}


bool Elevator::Stop() {
    return state_.stop;
}


}
