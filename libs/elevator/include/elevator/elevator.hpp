#pragma once

#include <string>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>
#include <control/requests.hpp>

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
        
        // Set
        void setDir(elev::common::MotorDir dir);
        void setMovement(elev::common::Movement mov);


        // Set lamps
        void setFloorIndicator();
        void setStopLamp(int value);
        void setDoorOpenLamp(int value);
        void setButtonLamp(int floor, elev::common::BtnType btn, int value);

        // Get
        elev::common::Movement getMovement();
        elev::common::MotorDir getMotorDir();
        int getFloor();


        // Get signals
        int getBtnSignal(int floor, elev::common::BtnType btn);
        int getFloorSensor(void);
        int getStopSignal(void);
        int getObs(void);



        
};

}

