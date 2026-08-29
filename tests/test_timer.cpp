#include <gtest/gtest.h>

#include <common/timer.hpp>

using elev::common::Timer;

TEST(Timer, StartsInactive) {
    Timer t;
    EXPECT_FALSE(t.Active());
    EXPECT_FALSE(t.Expired());
}

TEST(Timer, ActiveAfterStartAndNotExpiredForLongDuration) {
    Timer t;
    t.Start(100000);  // 100 s - safely longer than the test run
    EXPECT_TRUE(t.Active());
    EXPECT_FALSE(t.Expired());
}

TEST(Timer, ExpiresOnceTheDeadlineHasPassed) {
    Timer t;
    t.Start(0);  // deadline == now, so it is immediately reached
    EXPECT_TRUE(t.Active());
    EXPECT_TRUE(t.Expired());
}

TEST(Timer, StopClearsActiveAndExpired) {
    Timer t;
    t.Start(0);
    t.Stop();
    EXPECT_FALSE(t.Active());
    EXPECT_FALSE(t.Expired());  // an inactive timer never reports expired
}

TEST(Timer, RestartAfterExpiryBecomesPending) {
    Timer t;
    t.Start(0);
    EXPECT_TRUE(t.Expired());
    t.Start(100000);
    EXPECT_FALSE(t.Expired());
}