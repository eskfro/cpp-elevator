#pragma once

#include <common/types.hpp>

namespace elev::elevator {

class ElevatorState {
public:
    ElevatorState() = default;

    void Init();

    // Getters
    int Id() const;
    int Floor() const;
    int PrevFloor();
    bool Active() const;
    bool Obstruction() const;
    bool Fault() const;
    bool DoorOpen() const;
    bool Stopped() const;
    elev::common::MotorDir MotorDir() const;
    elev::common::MovingState MovingState() const;
    uint64_t Version() const;
    common::BoolTable Requests() const;
    common::Inertia Inertia() const;
    bool Valid() const;

    // Setters
    void SetId(int id);
    void SetFloor(int floor);
    void SetPrevFloor(int prev_floor);
    void SetStopped(bool stopped);
    void SetMotorDir(elev::common::MotorDir dir);
    void SetMovingState(elev::common::MovingState mov);
    void SetObstruction(bool obs);
    void SetFault(bool fault);
    void SetActivity(bool active);
    void SetDoorOpen(bool door_open);
    void SetRequests(common::BoolTable requests);
    void SetInertia(common::Inertia inertia);

    // Things
    void CopyFrom(const ElevatorState* rhs);
    void OnUpdate(const ElevatorState& state);
    void IncrementVersion() { version_++; }
    void OnWatchdogTimeout();
    bool Usable() const;

private:
    int id_{-1};
    int floor_{-1};
    bool active_{false};
    bool obstruction_{true};
    bool fault_{false};
    bool door_open_{false};
    bool stopped_{false};
    elev::common::MotorDir motor_dir_{0};
    elev::common::MovingState moving_state_{0};
    uint64_t version_{0};
    common::BoolTable requests_{};
    elev::common::Inertia inertia_{common::Inertia::None};
};

}  // namespace elev::elevator