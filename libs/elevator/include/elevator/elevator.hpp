#pragma once

#include <string>

// Libs
#include <common/config.hpp>
#include <common/types.hpp>

namespace elev::elevator {

class Elevator {
    public:
        Elevator();
        Elevator(int _ID, std::string _IP);

        void InitToFloor();

        // Door
        void OpenDoor();
        void CloseDoor();
        
        // Set
        void SetID(int _ID);
        void SetIP(std::string _IP);
        void SetFloor(int floor);
        void SetMotorDir(elev::common::MotorDir dir);
        void SetMovingState(elev::common::MovingState mov);
        void SetObs(bool obs);
        void SetActivity(bool active);
        void SetDoorOpen(bool door_open);
        void SetStop(bool stop);
        
        // Get
        int ID();
        std::string IP();
        int Floor();
        elev::common::MotorDir MotorDir();
        elev::common::MovingState MovingState();
        bool DoorOpen();
        bool Stop();
        bool Obs();

        // Lamps
        void SetDoorOpenLamp(int value);
        void SetFloorIndicator();
        void SetStopLamp(int value);
        void SetBtnLamp(int floor, elev::common::BtnType btn, int value);

        // Get signals
        int GetBtnSignal(int floor, elev::common::BtnType btn);
        int GetFloorSensor(void);
        int GetStopSignal(void);
        int GetObsSignal(void);

    private:
        elev::common::ElevatorState state_;

};

} // namespace elev::elevator

