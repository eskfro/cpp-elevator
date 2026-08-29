#include <gtest/gtest.h>

#include <common/config.hpp>
#include <common/types.hpp>
#include <ordersync/ordersync.hpp>

using elev::ordersync::Order;
using elev::ordersync::OrderTable;
using elev::common::OrderStatus;
using elev::common::BtnType;
namespace config = elev::config;

// --- Single Order state machine ---------------------------------------------

TEST(Order, DefaultState) {
    Order o;
    EXPECT_EQ(o.Status(), OrderStatus::None);
    EXPECT_EQ(o.Version(), 0u);
    EXPECT_EQ(o.AssignedId(), -1);
}

TEST(Order, RequestConfirmClearResetLifecycle) {
    Order o;

    o.OnRequest(1);
    EXPECT_EQ(o.Status(), OrderStatus::Requested);
    EXPECT_EQ(o.Version(), 1u);
    EXPECT_TRUE(o.ObservedBy(1));
    EXPECT_FALSE(o.ObservedBy(2));

    o.OnConfirm(2);
    EXPECT_EQ(o.Status(), OrderStatus::Confirmed);
    EXPECT_EQ(o.AssignedId(), 2);
    EXPECT_EQ(o.Version(), 2u);

    o.OnClear();
    EXPECT_EQ(o.Status(), OrderStatus::Clear);
    EXPECT_EQ(o.Version(), 3u);

    o.OnReset();
    EXPECT_EQ(o.Status(), OrderStatus::None);
    EXPECT_EQ(o.Version(), 4u);
}

TEST(Order, TransitionsAreGuardedByCurrentStatus) {
    Order o;

    // Can't confirm/clear/reset from None.
    o.OnConfirm(1);
    o.OnClear();
    o.OnReset();
    EXPECT_EQ(o.Status(), OrderStatus::None);
    EXPECT_EQ(o.Version(), 0u);

    // A second request while already Requested is a no-op.
    o.OnRequest(0);
    o.OnRequest(0);
    EXPECT_EQ(o.Status(), OrderStatus::Requested);
    EXPECT_EQ(o.Version(), 1u);
}

TEST(Order, RevokeSendsConfirmedBackToRequested) {
    Order o;
    o.OnRequest(0);
    o.OnConfirm(0);
    o.OnRevoke();
    EXPECT_EQ(o.Status(), OrderStatus::Requested);
    EXPECT_EQ(o.Version(), 3u);
}

// --- Valid --------------------------------------------------------------

TEST(Order, ValidAcceptsDefaultAndLifecycleStates) {
    Order o;
    EXPECT_TRUE(o.Valid());

    o.OnRequest(0);
    o.OnConfirm(config::kElevs - 1);
    o.OnClear();
    EXPECT_TRUE(o.Valid());
}

TEST(Order, ValidRejectsOutOfRangeAssignedId) {
    Order o;
    o.SetAssignedId(config::kElevs);
    EXPECT_FALSE(o.Valid());

    o.SetAssignedId(-2);
    EXPECT_FALSE(o.Valid());
}

TEST(OrderTable, ValidRejectsATableWithACorruptOrder) {
    OrderTable t;
    EXPECT_TRUE(t.Valid());

    t.Order(1, static_cast<int>(BtnType::HallUp))->SetAssignedId(99);
    EXPECT_FALSE(t.Valid());
}

// --- Merge logic (OnUpdate) ------------------------------------------------

TEST(Order, OnUpdateAdoptsStrictlyHigherVersion) {
    Order local;

    Order remote;
    remote.OnRequest(0);   // v1
    remote.OnConfirm(0);   // v2

    local.OnUpdate(remote);
    EXPECT_EQ(local.Status(), OrderStatus::Confirmed);
    EXPECT_EQ(local.Version(), 2u);
    EXPECT_EQ(local.AssignedId(), 0);
}

TEST(Order, OnUpdateIgnoresLowerVersion) {
    Order local;
    local.OnRequest(0);
    local.OnConfirm(0);  // v2, Confirmed

    Order stale;
    stale.OnRequest(1);  // v1, Requested

    local.OnUpdate(stale);
    EXPECT_EQ(local.Status(), OrderStatus::Confirmed);
    EXPECT_EQ(local.Version(), 2u);
    EXPECT_EQ(local.AssignedId(), 0);
}

TEST(Order, OnUpdateMergesObservationsAtEqualVersion) {
    Order a;
    a.OnRequest(0);  // v1, observed by 0

    Order b;
    b.OnRequest(1);  // v1, observed by 1

    a.OnUpdate(b);
    EXPECT_EQ(a.Version(), 1u);
    EXPECT_EQ(a.Status(), OrderStatus::Requested);
    EXPECT_TRUE(a.ObservedBy(0));
    EXPECT_TRUE(a.ObservedBy(1));
}

TEST(Order, OnUpdateBreaksConfirmTieWithLowestId) {
    Order a;
    a.OnRequest(0);
    a.OnConfirm(2);  // v2, Confirmed, assigned 2

    Order b;
    b.OnRequest(0);
    b.OnConfirm(1);  // v2, Confirmed, assigned 1

    a.OnUpdate(b);
    EXPECT_EQ(a.Status(), OrderStatus::Confirmed);
    EXPECT_EQ(a.Version(), 2u);
    EXPECT_EQ(a.AssignedId(), 1);  // lower id wins

    // Symmetric: the side that already has the lower id keeps it.
    b.OnUpdate(a);
    EXPECT_EQ(b.AssignedId(), 1);
}

// --- OrderTable -----------------------------------------------------------

TEST(OrderTable, ToBoolTableOnlyReturnsConfirmedOrdersForThisElevator) {
    OrderTable t;
    t.Order(2, static_cast<int>(BtnType::HallUp))->OnRequest(0);
    t.Order(2, static_cast<int>(BtnType::HallUp))->OnConfirm(0);

    auto mine = t.ToBoolTable(0);
    auto other = t.ToBoolTable(1);

    EXPECT_TRUE(mine[2][static_cast<int>(BtnType::HallUp)]);
    EXPECT_FALSE(other[2][static_cast<int>(BtnType::HallUp)]);

    // A merely requested order is not in anyone's bool table.
    t.Order(0, static_cast<int>(BtnType::HallDown))->OnRequest(0);
    EXPECT_FALSE(t.ToBoolTable(0)[0][static_cast<int>(BtnType::HallDown)]);
}

TEST(OrderTable, JoinMergesHallOrdersButLeavesCabOrdersLocal) {
    OrderTable local;
    OrderTable remote;

    remote.Order(1, static_cast<int>(BtnType::HallUp))->OnRequest(0);
    remote.Order(1, static_cast<int>(BtnType::HallUp))->OnConfirm(0);

    remote.Order(1, static_cast<int>(BtnType::Cab))->OnRequest(0);
    remote.Order(1, static_cast<int>(BtnType::Cab))->OnConfirm(0);

    local.Join(remote);

    EXPECT_EQ(local.Order(1, static_cast<int>(BtnType::HallUp))->Status(),
              OrderStatus::Confirmed);
    EXPECT_EQ(local.Order(1, static_cast<int>(BtnType::Cab))->Status(),
              OrderStatus::None);  // cab orders are never synced
}
