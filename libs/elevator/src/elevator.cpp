#include <elevator/elevator.hpp>

// Libs
#include <hardware/hardware.hpp> 

namespace elev::elevator {

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
}

void Elevator::closeDoor() {
    this->setDoorOpenLamp(0);
}


void Elevator::setStopLamp(int value) {
    elev::hardware::setStopLamp(value);
}   


void Elevator::setButtonLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::setBtnLamp(btn, floor, value);
}


void Elevator::setFloorIndicator() {
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
