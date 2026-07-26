#pragma once

#include <string>

// Libs
#include <elevator/elevator_state.hpp>
#include <common/config.hpp>
#include <common/types.hpp>
#include <buttons/button.hpp>

namespace elev::elevator {

class Elevator {
    public:
        Elevator() = delete;
        Elevator(int _ID, std::string _IP);

        bool HitNewFloor();
        void InitToFloor();
        void Step();

        // Getters
        ElevatorState* State();
        elev::buttons::ButtonMatrix* Buttons();

        // Door
        void OpenDoor();
        void CloseDoor();

        // Hardware
        void SetMotorDir(elev::common::MotorDir dir);
        void SetDoorOpenLamp(int value);
        void SetFloorIndicator();
        void SetStopLamp(int value);
        void SetButtonLamp(int floor, elev::common::BtnType btn, int value);

        // Get signals
        int FloorSensor(void);
        int StopSignal(void);
        int ObstructionSignal(void);

    private:
        int prev_floor_{};
        ElevatorState state_;
        elev::buttons::ButtonMatrix buttons_;

};

} // namespace elev::elevator

