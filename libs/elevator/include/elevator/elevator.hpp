#pragma once

#include <string>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>

namespace elev::elevator {

class Elevator {
    private:
        int ID;
        std::string IP; 

        int floor;
        bool is_obstruction;

        // Movement
        elev::common::MotorDir dir;  
        elev::common::Movement mov;

    public:
        Elevator(int _ID, std::string _IP);

        void openDoor();
        void closeDoor();
        void setDir(elev::common::MotorDir dir);
        
        // Set
        void setMovement(elev::common::Movement mov);

        // Get
        elev::common::Movement getMovement();
        elev::common::MotorDir getDir();
        int getFloor();

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

