#pragma once

#include <string>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>

namespace elev::elevator {

class Elevator {
    private:
        elev::common::ElevatorState state;

    public:
        Elevator();
        Elevator(int _ID, std::string _IP);

        void initToFloor();

        
        // Door
        void openDoor();
        void closeDoor();
        
        // Set
        void setID(int _ID);
        void setIP(std::string _IP);
        void setFloor(int floor);
        void setMotorDir(elev::common::MotorDir dir);
        void setMovingState(elev::common::MovingState mov);
        void setObs(bool obs);
        void setActivity(bool active);
        void setDoorOpen(bool door_open);
        void setStop(bool stop);
        
        // Get
        int getID();
        std::string getIP();
        int getFloor();
        elev::common::MotorDir getMotorDir();
        elev::common::MovingState getMovingState();
        bool getDoorOpen();
        bool getStop();
        bool getObs();

        // Lamps
        void setDoorOpenLamp(int value);
        void setFloorIndicator();
        void setStopLamp(int value);
        void setBtnLamp(int floor, elev::common::BtnType btn, int value);

        // Get signals
        int getBtnSignal(int floor, elev::common::BtnType btn);
        int getFloorSensor(void);
        int getStopSignal(void);
        int getObsSignal(void);



        
};

}

