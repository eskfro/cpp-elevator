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


// FSM Release Stop
ButtonFlags Controller::fsm_release_stop(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->getID() << " ] - FSM: Release Stop" << std::endl;
    ButtonFlags zeros{};
    
    elev->setStop(false);
    elev->setStopLamp(0);

    if (elev->getFloorSensor() == BETWEEN_FLOORS) {
        if (inertia == Inertia::UP) elev->setMotorDir(MotorDir::UP);
        else elev->setMotorDir(MotorDir::DOWN);

    } else {
        if (elev->getDoorOpen()) {
            doortimer.start(DOOR_OPEN_TIME_MS);
        } 

    }
}


// FSM Emergency Stop
ButtonFlags Controller::fsm_emergency_stop(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->getID() << " ] - FSM: Emergency Stop" << std::endl;
    ButtonFlags zero{};

    elev->setStop(true);
    elev->setMotorDir(MotorDir::STOP);
    elev->setStopLamp(1);

    if (elev->getFloorSensor() != BETWEEN_FLOORS) {
        elev->openDoor();
        doortimer.start(DOOR_OPEN_TIME_MS);
    }
    return zero;
}


// FSM Table Update
ButtonFlags Controller::fsm_table_update(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->getID() << " ] - FSM: Table Update" << std::endl;

    using namespace elev::common;

    DirMovPair pair;
    ButtonFlags zero{};
    int floor = elev->getFloor();

    switch (elev->getMovingState()) {

        case MovingState::DOOR_OPEN:
            if (shouldStop(floor)) {
                doortimer.start(DOOR_OPEN_TIME_MS);
                return clearCurrentFloor(floor);
            }
            return zero;
            
        case MovingState::MOVING:
            return zero;
        
        case MovingState::IDLE:

            pair = chooseDirection(floor);

            switch (elev->getMovingState()) {

                case MovingState::DOOR_OPEN:
                    if (elev->getFloor() != BETWEEN_FLOORS) {
                        elev->openDoor();
                    }
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    executeDecision(elev, pair);
                    return clearCurrentFloor(floor);

                case MovingState::MOVING:
                case MovingState::IDLE:
                    if (tryCloseDoor(elev)) {
                        executeDecision(elev, pair);
                    }
                    return zero;

                default:
                    return zero;
            }

        default:
            return zero;        
    }
    return zero;

}

// FSM Floor Arrival
ButtonFlags Controller::fsm_floor_arrival(elev::elevator::Elevator* elev) {
     std::cout << "[ Elevator "<< elev->getID() << " ] - FSM: Arrived @ Floor " << elev->getFloor() << std::endl; 
    using namespace elev::common;

    ButtonFlags zero{};
    int floor = elev->getFloor();
    elev->setFloorIndicator();

    switch (elev->getMovingState()) {

        case MovingState::MOVING:
            if (shouldStop(floor)) {
                elev->setMotorDir(MotorDir::STOP);
                elev->openDoor();
                doortimer.start(DOOR_OPEN_TIME_MS);
                return clearCurrentFloor(floor);
            } else {
                return zero;
            }
            
        default:
            return zero;
    }
    
}




// FSM Door Timeout
ButtonFlags Controller::fsm_door_timeout(elev::elevator::Elevator* elev) {
    std::cout << "[ Elevator "<< elev->getID() << " ] - FSM: Door Timeout" << std::endl;
    using namespace elev::common;
    
    DirMovPair pair;
    elev->setObs(elev->getObsSignal());
    ButtonFlags zero{};
    int floor = elev->getFloor();

    if (elev->getObs()) {
        std::cout << "OBS!" << std::endl;
        doortimer.start(DOOR_OPEN_TIME_MS);
        return zero;
    }
    
    switch(elev->getMovingState()) {

        case MovingState::DOOR_OPEN:

            pair =  chooseDirection(floor);
        
            switch (pair.mov) {

                case MovingState::DOOR_OPEN:
                    executeDecision(elev, pair);
                    doortimer.start(DOOR_OPEN_TIME_MS);
                    return clearCurrentFloor(elev->getFloor());
                
                case MovingState::MOVING:
                case MovingState::IDLE:
                    if (tryCloseDoor(elev)) {
                        executeDecision(elev, pair);
                    }
                    return zero;
            }

        default:
            return zero;
    }
}


void Controller::executeDecision(elev::elevator::Elevator* elev, DirMovPair pair) {
    elev->setMotorDir(pair.dir);
    elev->setMovingState(pair.mov);
    setInertia(pair.dir);       
}


int Controller::tryCloseDoor(elev::elevator::Elevator* elev) {
    if (elev->getObs()) {
        if (doortimer.isExpired()) doortimer.start(DOOR_OPEN_TIME_MS);
        return 0;
    } else {
        elev->closeDoor();
        elev->setDoorOpen(false);
        return 1;
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

