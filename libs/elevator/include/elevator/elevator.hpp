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
        Elevator(int _ID, std::string _IP);

        void setInactive();

        // Door
        void openDoor();
        void closeDoor();
        
        // Set
        void setFloor(int floor);
        void setDir(elev::common::MotorDir dir);
        void setMovement(elev::common::Movement mov);

        void setDoorState(bool state);
        bool getDoorState();

        // Get
        elev::common::Movement getMovement();
        elev::common::MotorDir getDir();
        std::string getIP();
        int getFloor();
        int getID();

        // Lamps
        void setDoorOpenLamp(int value);
        void setFloorIndicator();
        void setStopLamp(int value);
        void setButtonLamp(int floor, elev::common::BtnType btn, int value);

        // Get signals
        int getBtnSignal(int floor, elev::common::BtnType btn);
        int getFloorSensor(void);
        int getStopSignal(void);
        int getObs(void);



        
};

}

