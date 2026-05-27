#include <control/controller.hpp>

namespace elev::control {


bool Controller::is_table_update(RequestTable prev_requests) {
    using namespace elev::common;
    if (requests.areEqual(prev_requests)) return true;
    else return false;
}


RequestTable Controller::getRequestTable() {
    return this->requests;
}


void Controller::setFromOrderSlice(elev::ordersync::OrderSlice slice) {
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


void Controller::fsm_table_update(elev::elevator::Elevator* elev) {
    using namespace elev::common;

    // Extract elev variables
    int floor = elev->getFloor();
    MotorDir dir = elev->getDir();

    switch (elev->getMovement()) {

    case Movement::DOOR_OPEN:

        if (shouldStop(floor, dir)) {
            elev->setMovement(Movement::DOOR_OPEN);

            // TODO:
            // doorTimer.Start()

            // Clearing floor
            ButtonFlags b2c;
            b2c = clearCurrentFloor(floor, dir);

            // TODO:
            // return b2c; (notify order control to set (floor, btn) clear)
            break;

        } else {

            // TODO
            // if (doorTimer.isRunning()) {
            //      return Ø      
            // }

            elev->closeDoor();
            
            // Choose next output
            DirMovPair pair;
            pair = chooseDirection(floor, dir);

            // Set output
            elev->setDir(pair.dir);
            elev->setMovement(pair.mov);
            break;

        }   

    case Movement::MOVING:
        break;
    
    case Movement::IDLE:

        // Choose next output
        DirMovPair pair;
        pair = chooseDirection(floor, dir);
        
        // Set output
        elev->setDir(pair.dir);
        elev->setMovement(pair.mov);

        switch (elev->getMovement()) {

        case Movement::DOOR_OPEN:
            if (elev->getFloor() != BETWEEN_FLOORS) {
                elev->openDoor();
            }

            //doorTimer.start();

            ButtonFlags b2c;
            b2c = clearCurrentFloor(floor, dir);
            break;

        case Movement::MOVING:
            elev->closeDoor();
            break;

        case Movement::IDLE:
            break;
            
        }
        break;        
    }
}


void Controller::fsm_floor_arrival(elev::elevator::Elevator* elev, int floor) {
    using namespace elev::common;
    
    int floor = elev->getFloor();
    MotorDir dir = elev->getDir();

    elev->setFloorIndicator();

    switch (elev->getMovement()) {
    
    case Movement::MOVING:
        if (shouldStop(floor, dir)) {
            // Stop and open door
            elev->setDir(MotorDir::STOP);
            elev->openDoor();

            ButtonFlags b2c;
            b2c = clearCurrentFloor(floor, dir);

            // doorTimer.start();
            elev->setMovement(Movement::DOOR_OPEN);
        }
        break;
    default:
        break;
    }
    
}


void Controller::fsm_door_timeout(elev::elevator::Elevator* elev) {
    using namespace elev::common;

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
            //doorTimer.start();
            
            ButtonFlags b2c;
            b2c = clearCurrentFloor(elev->getFloor(), elev->getDir());
            break;

        case Movement::MOVING:
        case Movement::IDLE:
            elev->closeDoor();
            elev->setDir(elev->getDir());
            break;
        }
        break;
    
    default:
        break;
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

