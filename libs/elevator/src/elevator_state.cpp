
#include <elevator/elevator_state.hpp>
#include <common/types.hpp>

namespace elev::elevator {

// --- Getters ---

int ElevatorState::ID() {
    return ID_;
}

int ElevatorState::Floor() {
    return floor_;
}

bool ElevatorState::Active() {
    return active_;
}

bool ElevatorState::Obstruction() {
    return obstruction_;
}

bool ElevatorState::Fault() {
    return fault_;
}

bool ElevatorState::DoorOpen() {
    return door_open_;
}

bool ElevatorState::Stopped() {
    return stopped_;
}

elev::common::MotorDir ElevatorState::MotorDir() {
    return motor_dir_;
}

elev::common::MovingState ElevatorState::MovingState() {
    return moving_state_;
}

std::string ElevatorState::IP() {
    return IP_;
}

// --- Setters --- 

void ElevatorState::SetID(int ID) {
    ID_ = ID;
}

void ElevatorState::SetIP(std::string IP) {
    IP_ = IP;
}

void ElevatorState::SetFloor(int floor) {
    floor_ = floor;
}

void ElevatorState::SetStopped(bool stopped) {
    stopped_ = stopped;
}

void ElevatorState::SetMotorDir(elev::common::MotorDir dir) {
    motor_dir_ = dir;
}

void ElevatorState::SetMovingState(elev::common::MovingState mov) {
    moving_state_ = mov;
}

void ElevatorState::SetObs(bool obs) {
    obstruction_ = obs;
}

void ElevatorState::SetActivity(bool active) {
    active_ = active;
}

void ElevatorState::SetDoorOpen(bool door_open) {
    door_open_ = door_open;
}

} // namespace elev::elevator