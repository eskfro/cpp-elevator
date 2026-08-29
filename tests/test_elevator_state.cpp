#include <gtest/gtest.h>

#include <common/config.hpp>
#include <common/types.hpp>
#include <elevator/elevator_state.hpp>

using elev::elevator::ElevatorState;
using elev::common::MotorDir;
using elev::common::MovingState;
using elev::common::Inertia;
namespace config = elev::config;

TEST(ElevatorState, DefaultConstructed) {
    ElevatorState s;
    EXPECT_EQ(s.Id(), -1);
    EXPECT_EQ(s.Floor(), -1);
    EXPECT_FALSE(s.Active());
    EXPECT_EQ(s.Version(), 0u);
}

TEST(ElevatorState, InitResetsToKnownState) {
    ElevatorState s;
    s.Init();
    EXPECT_TRUE(s.Active());
    EXPECT_FALSE(s.Fault());
    EXPECT_FALSE(s.Stopped());
    EXPECT_FALSE(s.DoorOpen());
    EXPECT_EQ(s.MotorDir(), MotorDir::Stop);
    EXPECT_EQ(s.MovingState(), MovingState::Idle);
    EXPECT_EQ(s.Inertia(), Inertia::None);
    EXPECT_EQ(s.Version(), 1u);
}

TEST(ElevatorState, SetFloorRejectsOutOfRange) {
    ElevatorState s;
    s.SetFloor(2);
    EXPECT_EQ(s.Floor(), 2);

    s.SetFloor(-1);
    EXPECT_EQ(s.Floor(), 2);  // unchanged

    s.SetFloor(config::kFloors);
    EXPECT_EQ(s.Floor(), 2);  // unchanged
}

TEST(ElevatorState, IncrementVersion) {
    ElevatorState s;
    s.Init();
    s.IncrementVersion();
    s.IncrementVersion();
    EXPECT_EQ(s.Version(), 3u);
}

TEST(ElevatorState, OnUpdateAppliesStrictlyNewerState) {
    ElevatorState local;
    local.Init();  // version 1

    ElevatorState incoming;
    incoming.SetId(7);
    incoming.SetActivity(true);
    incoming.IncrementVersion();
    incoming.IncrementVersion();  // version 2

    local.OnUpdate(incoming);
    EXPECT_EQ(local.Id(), 7);
    EXPECT_EQ(local.Version(), 2u);
}

TEST(ElevatorState, OnUpdateIgnoresStaleOrEqualState) {
    ElevatorState local;
    local.Init();
    local.SetId(3);
    local.IncrementVersion();  // version 2

    ElevatorState stale;
    stale.SetId(99);  // version 0

    local.OnUpdate(stale);
    EXPECT_EQ(local.Id(), 3);
    EXPECT_EQ(local.Version(), 2u);
}

TEST(ElevatorState, CopyFromDuplicatesEverything) {
    ElevatorState src;
    src.Init();
    src.SetId(2);
    src.SetFloor(3);
    src.SetObstruction(true);
    src.SetInertia(Inertia::Up);

    ElevatorState dst;
    dst.CopyFrom(&src);
    EXPECT_EQ(dst.Id(), 2);
    EXPECT_EQ(dst.Floor(), 3);
    EXPECT_TRUE(dst.Obstruction());
    EXPECT_EQ(dst.Inertia(), Inertia::Up);
    EXPECT_EQ(dst.Version(), src.Version());
}

TEST(ElevatorState, ValidRejectsUnsetIdentity) {
    ElevatorState s;
    s.Init();
    EXPECT_FALSE(s.Valid());  // id / floor still -1
}

TEST(ElevatorState, ValidAcceptsAConsistentState) {
    ElevatorState s;
    s.Init();
    s.SetId(0);
    s.SetFloor(0);
    EXPECT_TRUE(s.Valid());
}

TEST(ElevatorState, ValidRejectsOutOfRangeIdentity) {
    ElevatorState s;
    s.Init();
    s.SetId(config::kElevs);
    s.SetFloor(0);
    EXPECT_FALSE(s.Valid());
}

TEST(ElevatorState, WatchdogTimeoutMarksInactive) {
    ElevatorState s;
    s.Init();
    s.IncrementVersion();

    s.OnWatchdogTimeout();
    EXPECT_FALSE(s.Active());
    EXPECT_EQ(s.Version(), 0u);
}