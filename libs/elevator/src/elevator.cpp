#include <elevator/elevator.hpp>
#include <hardware/hardware.hpp>

namespace elev::elevator {

Elevator::Elevator(int _ID, std::string _IP) {
    this->ID = _ID;
    this->IP = _IP;
}


void Elevator::setNewDir(elev::common::MotorDir next_dir) {
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


void Elevator::setAllButtonLamps(elev::ordercontrol::OrderTable ot) {
    using namespace elev::config;
    using namespace elev::common;

    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {
            int value;
            if (ot.getStatusAt(this->ID, f, b) == OrderStatus::CONF) {
                value = 1;
            } else {
                value = 0;
            }
            this->setButtonLamp(f, static_cast<BtnType>(b), value);
        }
    }
}


void Elevator::setButtonLamp(int floor, elev::common::BtnType btn, int value) {
    elev::hardware::setBtnLamp(btn, floor, value);
}


void Elevator::setFloorIndicator(int floor) {
    elev::hardware::setFloorIndicator(floor);
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



}
