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

// FSM Table Update
ButtonFlags Controller::fsm_table_update(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->getID() << " ] - Table Update" << std::endl;

    using namespace elev::common;

    ButtonFlags b2c{};
    int floor = elev->getFloor();

    switch (elev->getMovingState()) {

        case MovingState::DOOR_OPEN:
            if (shouldStop(floor)) {
                elev->setMovingState(MovingState::DOOR_OPEN);
                doortimer.start(DOOR_OPEN_TIME_MS);
                b2c = clearCurrentFloor(floor);
                return b2c;
            }

            if (doortimer.isActive()) {
                return b2c;
                
            } else {
                elev->closeDoor();
                DirMovPair pair;
                pair = chooseDirection(floor);
                elev->setMotorDir(pair.dir);
                elev->setMovingState(pair.mov);
                setInertia(elev->getMotorDir());
                return b2c;
            }
               

        case MovingState::MOVING:
            return b2c;
        
        case MovingState::IDLE:
            DirMovPair pair;
            pair = chooseDirection(floor);
            elev->setMotorDir(pair.dir);
            elev->setMovingState(pair.mov);
            setInertia(elev->getMotorDir());

            switch (elev->getMovingState()) {
                case MovingState::DOOR_OPEN:
                    if (elev->getFloor() != BETWEEN_FLOORS) {
                        elev->openDoor();
                    }
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(floor);
                    return b2c;

                case MovingState::MOVING:
                    elev->closeDoor();
                    return b2c;

                case MovingState::IDLE:
                    return b2c;
                
            }
            return b2c;
        return b2c;        
    }
    return b2c;
}

// FSM Floor Arrival
ButtonFlags Controller::fsm_floor_arrival(elev::elevator::Elevator* elev) {
     std::cout << "[ Elevator "<< elev->getID() << " ] - Arrived @ Floor " << elev->getFloor() << std::endl; 
    using namespace elev::common;
    
    ButtonFlags b2c{};
    int floor = elev->getFloor();
    elev->setFloorIndicator();

    switch (elev->getMovingState()) {
        case MovingState::MOVING:
            if (shouldStop(floor)) {
                elev->setMotorDir(MotorDir::STOP);
                elev->openDoor();
                doortimer.start(DOOR_OPEN_TIME_MS);
                b2c = clearCurrentFloor(floor);
                elev->setMovingState(MovingState::DOOR_OPEN);
            }
            return b2c;
            
        default:
            return b2c;
    }
    
}

// FSM Door Timeout
ButtonFlags Controller::fsm_door_timeout(elev::elevator::Elevator* elev) {
     std::cout << "[ Elevator "<< elev->getID() << " ] - Door Timeout" << std::endl;
    using namespace elev::common;

    ButtonFlags b2c{};
    int floor = elev->getFloor();
    
    switch(elev->getMovingState()) {
        case MovingState::DOOR_OPEN:
            DirMovPair pair;
            pair = chooseDirection(floor);
            elev->setMotorDir(pair.dir);
            elev->setMovingState(pair.mov);
            setInertia(elev->getMotorDir());
            
            switch (pair.mov) {
                case MovingState::DOOR_OPEN:
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    b2c = clearCurrentFloor(elev->getFloor());
                    return b2c;

                case MovingState::MOVING:
                case MovingState::IDLE:
                    elev->closeDoor();
                    elev->setMotorDir(elev->getMotorDir());
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
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::DOWN, MovingState::DOOR_OPEN};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
            else return {MotorDir::STOP, MovingState::IDLE};

        case Inertia::DOWN:
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
            if (requests.isRequestHere(floor))  return {MotorDir::UP,   MovingState::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
            else return {MotorDir::STOP, MovingState::IDLE};
            
        case Inertia::NONE:
            if (requests.isRequestHere(floor))  return {MotorDir::STOP, MovingState::DOOR_OPEN};
            if (requests.isRequestAbove(floor)) return {MotorDir::UP,   MovingState::MOVING};
            if (requests.isRequestBelow(floor)) return {MotorDir::DOWN, MovingState::MOVING};
            else return {MotorDir::STOP, MovingState::IDLE};

        default:
            return {MotorDir::STOP, MovingState::IDLE};
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

