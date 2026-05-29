#include <thread>
#include <chrono> 

#include <elevator/elevator.hpp>

// Libs
#include <hardware/hardware.hpp>

namespace elev::elevator {


Elevator::Elevator() {
    state.active = true;
}


void Elevator::initToFloor() {
    setMotorDir(elev::common::MotorDir::DOWN);

    while (getFloorSensor() == BETWEEN_FLOORS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    setMotorDir(elev::common::MotorDir::STOP);
    setFloor(getFloorSensor());
    setFloorIndicator();

    std::cout << "[ Elevator " << state.ID << " ] - INIT to floor " << state.floor << std::endl;
}


void Elevator::setIP(std::string _IP) {
    state.IP = _IP;
}


void Elevator::setID(int _ID) {
    state.ID = _ID;
}


void Elevator::setObs(bool obstruction) {
    this->state.obstruction = obstruction;
}


bool Elevator::getDoorState() {
    return this->state.door_open;
}


void Elevator::setDoorState(bool door_open) {
    this->state.door_open = door_open;
}


void Elevator::setFloor(int floor) {
    state.floor = floor;
}


Elevator::Elevator(int _ID, std::string _IP) {
    state.ID = _ID;
    state.IP = _IP;
    state.active = true;
}


// TODO: idk
void Elevator::setActivity(bool active) {
    state.active = active;
};


int Elevator::getID() {
    return state.ID;
}


std::string Elevator::getIP() {
    return state.IP;
}


void Elevator::setMotorDir(elev::common::MotorDir dir) {
    using namespace elev::common;
    MotorDir newDir;

    if (state.obstruction) {
        newDir = MotorDir::STOP;
    } else {
        newDir = dir;
    }

    elev::hardware::set_motor_dir(newDir);
    state.dir = newDir;
};


void Elevator::setMovingState(elev::common::MovingState mov) {
    state.mov = mov;
}


void Elevator::setDoorOpenLamp(int value) {
    elev::hardware::set_door_open_lamp(value);
}


void Elevator::openDoor() {
    this->setDoorOpenLamp(1);
    this->state.door_open = true;
}


void Elevator::closeDoor() {
    this->setDoorOpenLamp(0);
    this->state.door_open = false;
}


void Elevator::setStopLamp(int value) {
    elev::hardware::set_stop_lamp(value);
}   


void Elevator::setBtnLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::set_btn_lamp(btn, floor, value);
}


void Elevator::setFloorIndicator() {
    if (state.floor == BETWEEN_FLOORS) return;
    elev::hardware::set_floor_indicator(state.floor);
}


int Elevator::getBtnSignal(int floor, elev::common::BtnType btn) {
    return elev::hardware::get_btn_signal(btn, floor);
}


int Elevator::getFloorSensor() {
    return elev::hardware::get_floor_sensor();
}


int Elevator::getStopSignal() {
    return elev::hardware::get_stop_signal();
}


int Elevator::getObsSignal() {
    return elev::hardware::get_obs_signal();
}


elev::common::MovingState Elevator::getMovingState() {
    return state.mov;
}


int Elevator::getFloor() {
    return state.floor;
}


elev::common::MotorDir Elevator::getMotorDir() {
    return state.dir;
}


}
