#pragma once

#include <string>

#include <common/config.hpp>
#include <common/types.hpp>

#include <ordercontrol/ordercontrol.hpp>
#include <movement/requests.hpp>

namespace elev::elevator {

class Elevator {
    private:
        int ID;
        std::string IP; 

        int floor;
        bool is_obstruction;

        // Local requests
        elev::movement::RequestTable requests;
        elev::ordercontrol::OrderTable ot;


        elev::common::MotorDir dir;  
        elev::common::Movement mov;

    public:
        Elevator(int _ID, std::string _IP);
        
        // Set motor
        void setNewDir(elev::common::MotorDir dir);

        // Set lamp
        void setFloorIndicator(int floor);
        void setStopLamp(int value);
        void setDoorOpenLamp(int value);
        void setButtonLamp(int floor, elev::common::BtnType btn, int value);
        void setAllButtonLamps(elev::ordercontrol::OrderTable ot);

        // Get signals
        int getBtnSignal(int floor, elev::common::BtnType btn);
        int getFloorSensor(void);
        int getStopSignal(void);
        int getObs(void);



        
};

}

