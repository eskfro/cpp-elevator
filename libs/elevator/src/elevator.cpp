#include <elevator/elevator.hpp>

namespace elev::elevator {

Elevator::Elevator(int _ID, std::string _IP) {
    this->ID = _ID;
    this->IP = _IP;
}


void Elevator::setDir(elev::common::MotorDir next_dir) {
    using namespace elev::common;
    using namespace elev::hardware;
    MotorDir newDir;

    if (this->is_obstruction) {
        newDir = MotorDir::STOP;
    } else {
        newDir = next_dir;
    }

    setMotorDir(newDir);
    this->dir = newDir;
};


void Elevator::setMovement(elev::common::Movement mov) {
    this->mov = mov;
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
    elev::hardware::setFloorIndicator(this->floor);
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
    return this->mov;
}


int Elevator::getFloor() {
    return this->floor;
}


elev::common::MotorDir Elevator::getMotorDir() {
    return this->dir;
}


}
