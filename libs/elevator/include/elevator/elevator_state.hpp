#pragma once

#include <array>
#include <common/types.hpp>

#include "common/config.hpp"

namespace elev::elevator {

class ElevatorState {
public:
    ElevatorState() = default;

    void Init();

    // Getters
    int Id();
    int Floor();
    int PrevFloor();
    bool Active();
    bool Obstruction();
    bool Fault();
    bool DoorOpen();
    bool Stopped();
    elev::common::MotorDir MotorDir();
    elev::common::MovingState MovingState();
    uint64_t Version();
    std::array<std::array<bool, config::kButtons>, config::kFloors> Requests();
    common::Inertia Inertia();

    // Setters
    void SetId(int id);
    void SetFloor(int floor);
    void SetPrevFloor(int prev_floor);
    void SetStopped(bool stopped);
    void SetMotorDir(elev::common::MotorDir dir);
    void SetMovingState(elev::common::MovingState mov);
    void SetObstruction(bool obs);
    void SetActivity(bool active);
    void SetDoorOpen(bool door_open);
    void SetRequests(
        std::array<std::array<bool, config::kButtons>, config::kFloors>
            requests);
    void SetInertia(common::Inertia inertia);

    // Things
    void CopyFrom(ElevatorState* rhs);
    void OnUpdate(ElevatorState state);
    void IncrementVersion() { version_++; }

private:
    int id_{-1};
    int floor_{-1};
    int prev_floor_{-1};
    bool active_{false};
    bool obstruction_{true};
    bool fault_{false};
    bool door_open_{false};
    bool stopped_{false};
    elev::common::MotorDir motor_dir_{0};
    elev::common::MovingState moving_state_{0};
    uint64_t version_{0};
    std::array<std::array<bool, config::kButtons>, config::kFloors> requests_{};
    elev::common::Inertia inertia_{common::Inertia::None};
};

}  // namespace elev::elevator