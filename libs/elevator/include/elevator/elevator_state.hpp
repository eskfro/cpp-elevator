#pragma once

#include "common/config.hpp"
#include <array>
#include <common/types.hpp>

namespace elev::elevator {

class ElevatorState {

    public:
        ElevatorState() = default;

        // Getters
        int ID();
        int Floor();
        int PrevFloor();
        bool Active();
        bool Obstruction();
        bool Fault();
        bool DoorOpen();
        bool Stopped();
        elev::common::MotorDir MotorDir();
        elev::common::MovingState MovingState();
        std::string IP();
        uint64_t Version();
        std::array<std::array<bool, config::N_BUTTONS>, config::N_FLOORS> Requests();

        // Setters
        void SetID(int ID);
        void SetIP(std::string IP);
        void SetFloor(int floor);
        void SetPrevFloor(int prev_floor);
        void SetStopped(bool stopped);
        void SetMotorDir(elev::common::MotorDir dir);
        void SetMovingState(elev::common::MovingState mov);
        void SetObstruction(bool obs);
        void SetActivity(bool active);
        void SetDoorOpen(bool door_open);
        void SetVersion(uint64_t version);
        void SetRequests(std::array<std::array<bool, config::N_BUTTONS>, config::N_FLOORS> requests);


        // things
        void CopyFrom(ElevatorState* rhs);
        void OnUpdate(ElevatorState state);
        void IncrementVersion() { version_++; }


    private:
        int ID_;
        int floor_;
        int prev_floor_;
        bool active_;
        bool obstruction_;
        bool fault_;
        bool door_open_;
        bool stopped_;
        elev::common::MotorDir motor_dir_;
        elev::common::MovingState moving_state_;
        std::string IP_;
        uint64_t version_{};
        std::array<std::array<bool, config::N_BUTTONS>, config::N_FLOORS> requests_;

};

} // namespace elev::elevator