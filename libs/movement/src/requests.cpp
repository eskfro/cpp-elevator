#include <movement/requests.hpp>

namespace elev::movement {

void RequestTable::setValueAt(int floor, elev::common::BtnType btn, bool value) {
    this->table[floor][(int)btn] = value;
}

bool RequestTable::getValueAt(int floor, elev::common::BtnType btn) {
    return table[floor][(int)btn];
}

bool RequestTable::isRequestHere(int floor) {
    for (int b = 0; b < elev::config::N_BUTTONS; b++) if (this->table[floor][b]) return true;
    return false;
}

bool RequestTable::isRequestAbove(int floor) {
    for (int f = floor+1; f < elev::config::N_FLOORS; f++) if (isRequestHere(f)) return true;
    return false;
}

bool RequestTable::isRequestBelow(int floor) {
    for (int d_f = floor-1; d_f >= 0; d_f--) if (isRequestHere(d_f)) return true;
    return false;
}

elev::common::DirMovPair RequestTable::chooseDirection(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;
    switch (dir) {
        case MotorDir::UP:
            if (isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (isRequestHere(floor))  return {MotorDir::DOWN, Movement::DOOR_OPEN};
            if (isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            return                            {MotorDir::STOP, Movement::IDLE};

        case MotorDir::DOWN:
            if (isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            if (isRequestHere(floor))  return {MotorDir::UP,   Movement::DOOR_OPEN};
            if (isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            return                            {MotorDir::STOP, Movement::IDLE};
            
        case MotorDir::STOP:
            if (isRequestHere(floor))  return {MotorDir::STOP, Movement::DOOR_OPEN};
            if (isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            return                            {MotorDir::STOP, Movement::IDLE};

        default:
            return {MotorDir::STOP, Movement::IDLE};
    }
}

bool RequestTable::shouldStop(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;
    switch (dir) {
    case(MotorDir::DOWN):
        return table[floor][(int)BtnType::HALL_DOWN] || table[floor][(int)BtnType::CAB] || !isRequestBelow(floor);
    case(MotorDir::UP):
        return table[floor][(int)BtnType::HALL_UP] || table[floor][(int)BtnType::CAB] || !isRequestAbove(floor);
    case(MotorDir::STOP):
    default:
        return true;
    }
}


bool RequestTable::shouldClearImmediately(int floor, elev::common::MotorDir dir, int btnFloor, elev::common::BtnType btn) {
    using namespace elev::common;
    return floor == btnFloor &&
    (
        (dir == MotorDir::UP   && btn == BtnType::HALL_UP)   || 
        (dir == MotorDir::DOWN && btn == BtnType::HALL_DOWN) ||
        (dir == MotorDir::STOP)                              ||
        (btn == BtnType::CAB) 

    );
}

void RequestTable::clearCurrentFloor(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;

    // Always clear cab
    setValueAt(floor, BtnType::CAB, false);

    // Clearing hall calls
    switch(dir) {
        case MotorDir::UP:
            if (!isRequestAbove(floor) && !getValueAt(floor, BtnType::HALL_UP)) {
                setValueAt(floor, BtnType::HALL_DOWN, false);
            }
            setValueAt(floor, BtnType::HALL_UP, false);
            break;

        case MotorDir::DOWN:
            if (!isRequestBelow(floor) && !getValueAt(floor, BtnType::HALL_DOWN)) {
                setValueAt(floor, BtnType::HALL_UP, false);
            }
            setValueAt(floor, BtnType::HALL_DOWN, false);
            break;

        case MotorDir::STOP:
        default:
            setValueAt(floor, BtnType::HALL_UP, false);
            setValueAt(floor, BtnType::HALL_DOWN, false);
    }



}

}