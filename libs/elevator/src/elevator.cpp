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
    setDir(elev::common::MotorDir::DOWN);

    while (getFloorSensor() == BETWEEN_FLOORS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    setDir(elev::common::MotorDir::STOP);
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


void Elevator::setObs(bool value) {
    this->state.obstruction = value;
}


bool Elevator::getDoorState() {
    return this->state.door_open;
}


void Elevator::setDoorState(bool state) {
    this->state.door_open = state;
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
void Elevator::setInactive() {
    state.active = false;
};


int Elevator::getID() {
    return state.ID;
}


std::string Elevator::getIP() {
    return state.IP;
}


void Elevator::setDir(elev::common::MotorDir dir) {
    using namespace elev::common;
    MotorDir newDir;

    if (state.obstruction) {
        newDir = MotorDir::STOP;
    } else {
        newDir = dir;
    }

    elev::hardware::setMotorDir(newDir);
    state.dir = newDir;
};


void Elevator::setMovement(elev::common::Movement mov) {
    state.mov = mov;
}


void Elevator::setDoorOpenLamp(int value) {
    elev::hardware::setDoorOpenLamp(value);
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
    elev::hardware::setStopLamp(value);
}   


void Elevator::setBtnLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::setBtnLamp(btn, floor, value);
}


void Elevator::setFloorIndicator() {
    if (state.floor == BETWEEN_FLOORS) return;
    elev::hardware::setFloorIndicator(state.floor);
}


int Elevator::getBtnSignal(int floor, elev::common::BtnType btn) {
    return elev::hardware::getBtnSignal(btn, floor);
}


int Elevator::getFloorSensor() {
    return elev::hardware::getFloorSensor();
}


int Elevator::getStopSignal() {
    return elev::hardware::getStopSignal();
}


int Elevator::getObs() {
    return elev::hardware::getObs();
}


elev::common::Movement Elevator::getMovement() {
    return state.mov;
}


int Elevator::getFloor() {
    return state.floor;
}


elev::common::MotorDir Elevator::getDir() {
    return state.dir;
}


}
