#pragma once

#include <string>
#include <mutex>

// Libs
#include <elevator/elevator_state.hpp>
#include <common/config.hpp>
#include <common/types.hpp>
#include <buttons/button.hpp>

namespace elev::elevator {

class Elevator {
    public:
        Elevator() = delete;
        Elevator(int id, std::string ip);

        void Step();
        void Init();
        bool HitNewFloor();
        void InitToFloor();

        // Get
        ElevatorState* State();
        ElevatorState StateCopy();
        elev::buttons::ButtonTable* Buttons();

        // Door
        void OpenDoor();
        void CloseDoor();

        // Hardware
        void SetMotorDir(elev::common::MotorDir dir);
        void SetDoorOpenLamp(int value);
        void SetFloorIndicator();
        void SetStopLamp(int value);
        void SetButtonLamp(int floor, elev::common::BtnType btn, int value);

        // Hardware signals wrapper
        int FloorSensor(void);
        int StopSignal(void);
        int ObstructionSignal(void);

    private:
        ElevatorState state_;
        elev::buttons::ButtonTable buttons_;

};

} // namespace elev::elevator

