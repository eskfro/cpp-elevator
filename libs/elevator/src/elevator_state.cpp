
#include "common/config.hpp"
#include <cstdint>
#include <elevator/elevator_state.hpp>
#include <common/types.hpp>

namespace elev::elevator {

void ElevatorState::Init() {
    active_ = true;
    fault_ = false;
    stopped_ = false;

    door_open_ = false;

    motor_dir_ = common::MotorDir::Stop;
    moving_state_ = common::MovingState::Idle;
    inertia_ = common::Inertia::None;

    version_ = 1;
}

// --- Getters ---

int ElevatorState::Id() {
    return id_;
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

uint64_t ElevatorState::Version() {
    return version_;
}

std::array<std::array<bool, config::kButtons>, config::kFloors> ElevatorState::Requests() {
    return requests_;
}

common::Inertia ElevatorState::Inertia() {
    return inertia_;
}

// --- Setters --- 

void ElevatorState::SetId(int id) {
    id_ = id;
}

void ElevatorState::SetFloor(int floor) {
    if (floor < 0 || floor >= config::kFloors) return;
    floor_ = floor;
}

void ElevatorState::SetPrevFloor(int prev_floor) {
    if (prev_floor < 0 || prev_floor >= config::kFloors) return;
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

void ElevatorState::SetVersion(uint64_t version) {
    version_ = version;
}

void ElevatorState::CopyFrom(ElevatorState* rhs) {
    id_ = rhs->Id();
    floor_ = rhs->Floor();
    prev_floor_ = rhs->PrevFloor();
    active_ = rhs->Active();
    obstruction_ = rhs->Obstruction();
    fault_ = rhs->Fault();
    door_open_ = rhs->DoorOpen();
    stopped_ = rhs->Stopped();
    motor_dir_ = rhs->MotorDir();
    moving_state_ = rhs->MovingState();
    version_ = rhs->Version();
    requests_ = rhs->Requests();
    inertia_ = rhs->Inertia();
}

void ElevatorState::OnUpdate(ElevatorState rcv) {
    if (rcv.Version() <= version_) return;

    CopyFrom(&rcv);
}

void ElevatorState::SetRequests(std::array<std::array<bool, config::kButtons>, config::kFloors> requests) {
    requests_ = requests;
}

void ElevatorState::SetInertia(common::Inertia inertia) {
    inertia_ = inertia;
}

} // namespace elev::elevator