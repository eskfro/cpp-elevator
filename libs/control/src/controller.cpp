#include <control/controller.hpp>

namespace elev::control {


Controller::Controller() {
    doortimer.stop();
    inertia = Inertia::NONE;
}


DoorTimer* Controller::getDoorTimer() {
    return &doortimer;
}


void Controller::setInertia(MotorDir dir) {
    if (dir == MotorDir::UP) inertia = Inertia::UP;
    if (dir == MotorDir::DOWN) inertia = Inertia::DOWN;
}


RequestTable Controller::getRequests() {
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
    std::cout << "[ Elevator "<< elev->getID() << " ] - Table Update" << std::endl;

    using namespace elev::common;

    ButtonFlags b2c{};
    int floor = elev->getFloor();

    switch (elev->getMovement()) {
        case Movement::DOOR_OPEN:
            if (shouldStop(floor)) {
                elev->setMovement(Movement::DOOR_OPEN);
                doortimer.start(DOOR_OPEN_TIME_MS);
                b2c = clearCurrentFloor(floor);
                return b2c;

            } else {
                if (doortimer.isActive()) {
                    return b2c;
                } else {
                    elev->closeDoor();
                    DirMovPair pair;
                    pair = chooseDirection(floor);
                    elev->setDir(pair.dir);
                    elev->setMovement(pair.mov);
                    setInertia(elev->getDir());
                    return b2c;
                }
            }   

        case Movement::MOVING:
            return b2c;
        
        case Movement::IDLE:
            DirMovPair pair;
            pair = chooseDirection(floor);
            elev->setDir(pair.dir);
            elev->setMovement(pair.mov);
            setInertia(elev->getDir());

            switch (elev->getMovement()) {
                case Movement::DOOR_OPEN:
                    if (elev->getFloor() != BETWEEN_FLOORS) {
                        elev->openDoor();
                    }
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(floor);
                    return b2c;

                case Movement::MOVING:
                    elev->closeDoor();
                    return b2c;

                case Movement::IDLE:
                    return b2c;
                
            }
            return b2c;
        return b2c;        
    }
    return b2c;
}


ButtonFlags Controller::fsm_floor_arrival(elev::elevator::Elevator* elev) {
     std::cout << "[ Elevator "<< elev->getID() << " ] - arrived at floor " << elev->getFloor() << std::endl; 

    using namespace elev::common;
    
    ButtonFlags b2c{};
    int floor = elev->getFloor();

    elev->setFloorIndicator();

    switch (elev->getMovement()) {
        case Movement::MOVING:
            if (shouldStop(floor)) {
                elev->setDir(MotorDir::STOP);
                elev->openDoor();
                b2c = clearCurrentFloor(floor);
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
    
    switch(elev->getMovement()) {
        case Movement::DOOR_OPEN:
            DirMovPair pair;
            pair = chooseDirection(floor);
            elev->setDir(pair.dir);
            elev->setMovement(pair.mov);
            setInertia(elev->getDir());
            
            switch (pair.mov) {
                case Movement::DOOR_OPEN:
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(elev->getFloor());
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


elev::common::DirMovPair Controller::chooseDirection(int floor) {
    using namespace elev::common;
    switch (inertia) {
        case Inertia::UP:
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::DOWN, Movement::DOOR_OPEN};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};

        case Inertia::DOWN:
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::UP,   Movement::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};
            
        case Inertia::NONE:
            if (requests.isRequestHere(floor))  return {MotorDir::STOP, Movement::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   Movement::MOVING};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, Movement::MOVING};
            else return {MotorDir::STOP, Movement::IDLE};

        default:
            return {MotorDir::STOP, Movement::IDLE};
    }
};


bool Controller::shouldStop(int floor) {
    using namespace elev::common;
    switch (inertia) {
    case(Inertia::DOWN):
        return requests.getValueAt(floor, BtnType::HALL_DOWN) || requests.getValueAt(floor, BtnType::CAB) || !requests.isRequestBelow(floor);
    case(Inertia::UP):
        return requests.getValueAt(floor, BtnType::HALL_UP) || requests.getValueAt(floor, BtnType::CAB) || !requests.isRequestAbove(floor);
    case(Inertia::NONE):
    default:
        return true;
    }
}


bool Controller::shouldClearImmediately(int floor, int btnFloor, elev::common::BtnType btn) {
    using namespace elev::common;
    return floor == btnFloor &&
    (
        (inertia == Inertia::UP   && btn == BtnType::HALL_UP)   || 
        (inertia == Inertia::DOWN && btn == BtnType::HALL_DOWN) ||
        (btn == BtnType::CAB) 

    );
}


ButtonFlags Controller::clearCurrentFloor(int floor) {
    using namespace elev::common;

    // Buttons to clear
    ButtonFlags b2c{};

    // Always clear cab
    b2c[(int)BtnType::CAB] = true;

    // Clearing hall calls
    switch(inertia) {
        case Inertia::UP:
            if (!requests.isRequestAbove(floor) && !requests.getValueAt(floor, BtnType::HALL_UP)) {
                b2c[(int)BtnType::HALL_DOWN] = true;
            }
            b2c[(int)BtnType::HALL_UP] = true;
            return b2c;

        case Inertia::DOWN:
            if (!requests.isRequestBelow(floor) && !requests.getValueAt(floor, BtnType::HALL_DOWN)) {
                b2c[(int)BtnType::HALL_UP] = true;
            }
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

        default:
            b2c[(int)BtnType::HALL_UP] = true;
            b2c[(int)BtnType::HALL_DOWN] = true;
            return b2c;

    }



}

};

