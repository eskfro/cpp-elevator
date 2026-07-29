
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

int ElevatorState::PrevFloor() {
    return prev_floor_;
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

void ElevatorState::SetPrevFloor(int prev_floor) {
    prev_floor_ = prev_floor;
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

void ElevatorState::SetObstruction(bool obs) {
    obstruction_ = obs;
}

void ElevatorState::SetActivity(bool active) {
    active_ = active;
}

void ElevatorState::SetDoorOpen(bool door_open) {
    door_open_ = door_open;
}

void ElevatorState::CopyFrom(ElevatorState* rhs) {
    ID_ = rhs->ID();
    floor_ = rhs->Floor();
    prev_floor_ = rhs->PrevFloor();
    active_ = rhs->Active();
    obstruction_ = rhs->Obstruction();
    fault_ = rhs->Fault();
    door_open_ = rhs->DoorOpen();
    stopped_ = rhs->Stopped();
    motor_dir_ = rhs->MotorDir();
    moving_state_ = rhs->MovingState();
    IP_ = rhs->IP();
}

} // namespace elev::elevator