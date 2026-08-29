#include <gtest/gtest.h>

#include <common/config.hpp>
#include <common/types.hpp>
#include <control/requests.hpp>

using elev::control::RequestTable;
namespace config = elev::config;

TEST(RequestTable, DefaultsToEmpty) {
    RequestTable rt;
    for (int f = 0; f < config::kFloors; f++) {
        EXPECT_FALSE(rt.IsRequestHere(f));
        for (int b = 0; b < config::kButtons; b++) {
            EXPECT_FALSE(rt.Value(f, b));
        }
    }
}

TEST(RequestTable, SetAndReadBack) {
    RequestTable rt;
    rt.SetValue(2, 1, true);
    EXPECT_TRUE(rt.Value(2, 1));
    EXPECT_TRUE(rt.IsRequestHere(2));
    EXPECT_FALSE(rt.Value(2, 0));
    EXPECT_FALSE(rt.IsRequestHere(1));
}

TEST(RequestTable, OutOfRangeAccessIsIgnored) {
    RequestTable rt;

    // Writes outside the grid are silently dropped, no crash.
    rt.SetValue(-1, 0, true);
    rt.SetValue(config::kFloors, 0, true);
    rt.SetValue(0, -1, true);
    rt.SetValue(0, config::kButtons, true);

    // Reads outside the grid return false.
    EXPECT_FALSE(rt.Value(-1, 0));
    EXPECT_FALSE(rt.Value(config::kFloors, 0));
    EXPECT_FALSE(rt.IsRequestHere(-1));
    EXPECT_FALSE(rt.IsRequestHere(config::kFloors));

    // ...and nothing leaked into the valid range.
    EXPECT_FALSE(rt.IsRequestHere(0));
}

TEST(RequestTable, RequestAboveAndBelow) {
    RequestTable rt;
    rt.SetValue(3, static_cast<int>(elev::common::BtnType::Cab), true);

    EXPECT_TRUE(rt.IsRequestAbove(1));
    EXPECT_TRUE(rt.IsRequestAbove(2));
    EXPECT_FALSE(rt.IsRequestAbove(3));   // nothing above the top floor
    EXPECT_FALSE(rt.IsRequestBelow(3));   // the request is *at* floor 3, not below it
    EXPECT_FALSE(rt.IsRequestBelow(0));
}

TEST(RequestTable, RequestBelowSeesLowerFloors) {
    RequestTable rt;
    rt.SetValue(0, static_cast<int>(elev::common::BtnType::Cab), true);

    EXPECT_TRUE(rt.IsRequestBelow(1));
    EXPECT_TRUE(rt.IsRequestBelow(3));
    EXPECT_FALSE(rt.IsRequestBelow(0));
    EXPECT_FALSE(rt.IsRequestAbove(0));
}

TEST(RequestTable, TableReflectsWrites) {
    RequestTable rt;
    rt.SetValue(1, 0, true);
    auto table = rt.Table();
    EXPECT_TRUE(table[1][0]);
    EXPECT_FALSE(table[0][0]);
}