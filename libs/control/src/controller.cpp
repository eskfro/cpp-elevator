#include <control/controller.hpp>

namespace elev::control {


bool Controller::is_door_timeout(bool timer_active) {
    if (timer_active && doortimer.isExpired()) return true;
    return false;

}


bool Controller::is_table_update(RequestTable prev_requests) {
    if (requests.equal(prev_requests)) return true;
    else return false;
}


RequestTable Controller::getRequestTable() {
    return this->requests;
}


void Controller::updateRequests(elev::ordersync::OrderSlice slice) {
    using namespace elev::common;
    for (int f = 0; f < N_FLOORS; f++) {
        for (int b = 0; b < N_BUTTONS; b++) {

            if (slice.getValueAt(f, (BtnType)b) == OrderStatus::CONFIRMED) {
                requests.setValueAt(f, (BtnType)b, true);
            } else {
                requests.setValueAt(f, (BtnType)b, false);
            }
        }
    }
}


ButtonFlags Controller::fsm_table_update(elev::elevator::Elevator* elev) {
    using namespace elev::common;

    ButtonFlags b2c{};
    int floor = elev->getFloor();
    MotorDir dir = elev->getDir();

    switch (elev->getMovement()) {
        case Movement::DOOR_OPEN:
            if (shouldStop(floor, dir)) {
                elev->setMovement(Movement::DOOR_OPEN);
                doortimer.start(DOOR_OPEN_TIME_MS);
                b2c = clearCurrentFloor(floor, dir);
                return b2c;

            } else {
                if (doortimer.isActive()) {
                    return b2c;
                } else {
                    elev->closeDoor();
                    DirMovPair pair;
                    pair = chooseDirection(floor, dir);
                    elev->setDir(pair.dir);
                    elev->setMovement(pair.mov);
                    return b2c;
                }
            }   

        case Movement::MOVING:
            return b2c;
        
        case Movement::IDLE:
            DirMovPair pair;
            pair = chooseDirection(floor, dir);
            elev->setDir(pair.dir);
            elev->setMovement(pair.mov);

            switch (elev->getMovement()) {
                case Movement::DOOR_OPEN:
                    if (elev->getFloor() != BETWEEN_FLOORS) {
                        elev->openDoor();
                    }
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(floor, dir);
                    return b2c;

                case Movement::MOVING:
                    elev->closeDoor();
                    return b2c;

                case Movement::IDLE:
                    return b2c;
                
            }
            return b2c;        
    }
}


ButtonFlags Controller::fsm_floor_arrival(elev::elevator::Elevator* elev) {
    using namespace elev::common;
    
    ButtonFlags b2c{};
    int floor = elev->getFloor();
    MotorDir dir = elev->getDir();

    elev->setFloorIndicator();

    switch (elev->getMovement()) {
        case Movement::MOVING:
            if (shouldStop(floor, dir)) {
                elev->setDir(MotorDir::STOP);
                elev->openDoor();
                b2c = clearCurrentFloor(floor, dir);
                doortimer.start(DOOR_OPEN_TIME_MS);
                elev->setMovement(Movement::DOOR_OPEN);
            }
            return b2c;
            
        default:
            return b2c;
    }
    
}


ButtonFlags Controller::fsm_door_timeout(elev::elevator::Elevator* elev) {
    using namespace elev::common;

    ButtonFlags b2c{};
    int floor = elev->getFloor();
    MotorDir dir = elev->getDir();
    
    switch(elev->getMovement()) {
        case Movement::DOOR_OPEN:
            DirMovPair pair;
            pair = chooseDirection(floor, dir);
            elev->setDir(pair.dir);
            elev->setMovement(pair.mov);
            
            switch (pair.mov) {
                case Movement::DOOR_OPEN:
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(elev->getFloor(), elev->getDir());
                    return b2c;

                case Movement::MOVING:
                case Movement::IDLE:
                    elev->closeDoor();
                    elev->setDir(elev->getDir());
                    return b2c;
            }

            return b2c;
        
        default:
            return b2c;
    }
}


elev::common::DirMovPair Controller::chooseDirection(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;
    switch (dir) {
        case MotorDir::UP:
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::DOWN, Movement::DOOR_OPEN};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};

        case MotorDir::DOWN:
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::UP,   Movement::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};
            
        case MotorDir::STOP:
            if (requests.isRequestHere(floor))  return {MotorDir::STOP, Movement::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};

        default:
            return {MotorDir::STOP, Movement::IDLE};
    }
};


bool Controller::shouldStop(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;
    switch (dir) {
    case(MotorDir::DOWN):
        return requests.getValueAt(floor, BtnType::HALL_DOWN) || requests.getValueAt(floor, BtnType::CAB) || !requests.isRequestBelow(floor);
    case(MotorDir::UP):
        return requests.getValueAt(floor, BtnType::HALL_UP) || requests.getValueAt(floor, BtnType::CAB) || !requests.isRequestAbove(floor);
    case(MotorDir::STOP):
    default:
        return true;
    }
}


bool Controller::shouldClearImmediately(int floor, elev::common::MotorDir dir, int btnFloor, elev::common::BtnType btn) {
    using namespace elev::common;
    return floor == btnFloor &&
    (
        (dir == MotorDir::UP   && btn == BtnType::HALL_UP)   || 
        (dir == MotorDir::DOWN && btn == BtnType::HALL_DOWN) ||
        (dir == MotorDir::STOP)                              ||
        (btn == BtnType::CAB) 

    );
}


ButtonFlags Controller::clearCurrentFloor(int floor, elev::common::MotorDir dir) {
    using namespace elev::common;

    // Buttons to clear
    ButtonFlags b2c{};

    // Always clear cab
    requests.setValueAt(floor, BtnType::CAB, false);
    b2c[(int)BtnType::CAB] = true;

    // Clearing hall calls
    switch(dir) {
        case MotorDir::UP:
            if (!requests.isRequestAbove(floor) && !requests.getValueAt(floor, BtnType::HALL_UP)) {
                requests.setValueAt(floor, BtnType::HALL_DOWN, false);
                b2c[(int)BtnType::HALL_DOWN] = true;
            }
            requests.setValueAt(floor, BtnType::HALL_UP, false);
            b2c[(int)BtnType::HALL_UP] = true;
            return b2c;

        case MotorDir::DOWN:
            if (!requests.isRequestBelow(floor) && !requests.getValueAt(floor, BtnType::HALL_DOWN)) {
                requests.setValueAt(floor, BtnType::HALL_UP, false);
                b2c[(int)BtnType::HALL_UP] = true;
            }
            requests.setValueAt(floor, BtnType::HALL_DOWN, false);
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

        case MotorDir::STOP:
        default:
            requests.setValueAt(floor, BtnType::HALL_UP, false);
            requests.setValueAt(floor, BtnType::HALL_DOWN, false);
            b2c[(int)BtnType::HALL_UP] = true;
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

    }



}

};

