#include <gtest/gtest.h>

#include <initializer_list>
#include <tuple>

#include <common/config.hpp>
#include <common/types.hpp>
#include <control/controller.hpp>
#include <elevator/elevator.hpp>

using elev::control::Controller;
using elev::elevator::Elevator;
using elev::common::BoolTable;
using elev::common::BtnType;
using elev::common::MotorDir;
using elev::common::MovingState;

namespace {

int idx(BtnType b) { return static_cast<int>(b); }

// The controller only exposes inertia through SetInertia(Elevator*, dir).
// Constructing an Elevator is cheap and touches no hardware as long as we
// never call a method that sends to the simulator - SetInertia only writes
// into ElevatorState, so this is safe.
void set_inertia(Controller& c, MotorDir dir) {
    Elevator e(0, "");
    c.SetInertia(&e, dir);
}

BoolTable table_with(std::initializer_list<std::tuple<int, BtnType>> on) {
    BoolTable t{};
    for (auto [floor, btn] : on) t[floor][idx(btn)] = true;
    return t;
}

}  // namespace

// --- ChooseDirection ------------------------------------------------------

TEST(Controller, ChooseDirectionIdleWhenNoRequests) {
    Controller c;
    auto d = c.ChooseDirection(2);
    EXPECT_EQ(d.motor_dir, MotorDir::Stop);
    EXPECT_EQ(d.moving_state, MovingState::Idle);
}

TEST(Controller, ChooseDirectionOpensDoorForRequestOnCurrentFloor) {
    Controller c;
    c.SetRequests(table_with({{2, BtnType::Cab}}));
    auto d = c.ChooseDirection(2);
    EXPECT_EQ(d.motor_dir, MotorDir::Stop);
    EXPECT_EQ(d.moving_state, MovingState::DoorOpen);
}

TEST(Controller, ChooseDirectionMovesTowardsRequest) {
    Controller c;
    c.SetRequests(table_with({{3, BtnType::Cab}}));
    EXPECT_EQ(c.ChooseDirection(1).motor_dir, MotorDir::Up);

    c.SetRequests(table_with({{0, BtnType::Cab}}));
    EXPECT_EQ(c.ChooseDirection(2).motor_dir, MotorDir::Down);
}

TEST(Controller, ChooseDirectionKeepsMovingUpWhileHeadingUp) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    // Requests both above and below; upward inertia should win.
    c.SetRequests(table_with({{0, BtnType::Cab}, {3, BtnType::Cab}}));
    EXPECT_EQ(c.ChooseDirection(1).motor_dir, MotorDir::Up);
}

TEST(Controller, ChooseDirectionReversesWhenNothingLeftInInertiaDirection) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    c.SetRequests(table_with({{0, BtnType::Cab}}));  // only below
    EXPECT_EQ(c.ChooseDirection(2).motor_dir, MotorDir::Down);
}

// --- ShouldStop ---------------------------------------------------------

TEST(Controller, ShouldStopAlwaysTrueWithNoInertia) {
    Controller c;
    EXPECT_TRUE(c.ShouldStop(1));
}

TEST(Controller, ShouldStopForMatchingHallCallInTravelDirection) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    c.SetRequests(table_with({{2, BtnType::HallUp}, {3, BtnType::Cab}}));
    EXPECT_TRUE(c.ShouldStop(2));
}

TEST(Controller, ShouldNotStopForOppositeHallCallWhenMoreWorkAhead) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    // HallDown here, but there is still a request further up.
    c.SetRequests(table_with({{2, BtnType::HallDown}, {3, BtnType::Cab}}));
    EXPECT_FALSE(c.ShouldStop(2));
}

TEST(Controller, ShouldStopForOppositeHallCallWhenItIsTheLastRequest) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    c.SetRequests(table_with({{2, BtnType::HallDown}}));  // nothing above
    EXPECT_TRUE(c.ShouldStop(2));
}

// --- ShouldClearImmediately ------------------------------------------------

TEST(Controller, ShouldClearImmediately) {
    Controller c;
    set_inertia(c, MotorDir::Up);

    EXPECT_TRUE(c.ShouldClearImmediately(2, 2, BtnType::HallUp));
    EXPECT_TRUE(c.ShouldClearImmediately(2, 2, BtnType::Cab));
    EXPECT_FALSE(c.ShouldClearImmediately(2, 2, BtnType::HallDown));
    EXPECT_FALSE(c.ShouldClearImmediately(2, 3, BtnType::HallUp));  // different floor
}

// --- ClearCurrentFloor ---------------------------------------------------

TEST(Controller, ClearCurrentFloorClearsBothHallCallsWithNoInertia) {
    Controller c;
    auto b2c = c.ClearCurrentFloor(1);
    EXPECT_TRUE(b2c[idx(BtnType::Cab)]);
    EXPECT_TRUE(b2c[idx(BtnType::HallUp)]);
    EXPECT_TRUE(b2c[idx(BtnType::HallDown)]);
}

TEST(Controller, ClearCurrentFloorKeepsOppositeHallCallWhenMoreWorkAhead) {
    Controller c;
    set_inertia(c, MotorDir::Up);
    c.SetRequests(table_with({{3, BtnType::Cab}}));  // still work above

    auto b2c = c.ClearCurrentFloor(2);
    EXPECT_TRUE(b2c[idx(BtnType::HallUp)]);
    EXPECT_TRUE(b2c[idx(BtnType::Cab)]);
    EXPECT_FALSE(b2c[idx(BtnType::HallDown)]);  // leave the down call for the way back
}

TEST(Controller, ClearCurrentFloorClearsOppositeHallCallWhenNothingAhead) {
    Controller c;
    set_inertia(c, MotorDir::Up);  // no requests above

    auto b2c = c.ClearCurrentFloor(2);
    EXPECT_TRUE(b2c[idx(BtnType::HallUp)]);
    EXPECT_TRUE(b2c[idx(BtnType::HallDown)]);
}

// --- RequestTableUpdated -------------------------------------------------

TEST(Controller, RequestTableUpdatedDetectsChangesOnce) {
    Controller c;
    EXPECT_FALSE(c.RequestTableUpdated());  // nothing has changed yet

    c.SetRequests(table_with({{1, BtnType::HallUp}}));
    EXPECT_TRUE(c.RequestTableUpdated());   // change observed
    EXPECT_FALSE(c.RequestTableUpdated());  // ...and only reported once
}

TEST(Controller, SetRequestsIsVisibleThroughRequests) {
    Controller c;
    c.SetRequests(table_with({{1, BtnType::HallUp}}));
    EXPECT_TRUE(c.Requests().Value(1, idx(BtnType::HallUp)));
    EXPECT_FALSE(c.Requests().Value(0, idx(BtnType::HallUp)));
}
