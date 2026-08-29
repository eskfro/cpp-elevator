#include <gtest/gtest.h>

#include <common/config.hpp>
#include <elevator/elevator_state.hpp>
#include <network/udp_bcast.hpp>
#include <ordersync/ordersync.hpp>

using elev::elevator::ElevatorState;
using elev::network::NetworkPacket;
namespace config = elev::config;

namespace {

ElevatorState consistent_state(int id, int floor) {
    ElevatorState s;
    s.Init();
    s.SetId(id);
    s.SetFloor(floor);
    return s;
}

}  // namespace

TEST(NetworkPacket, ValidAcceptsAWellFormedPacket) {
    ElevatorState state = consistent_state(1, 2);
    elev::ordersync::OrderTable orders;

    NetworkPacket packet;
    packet.Init(&orders, &state, nullptr);

    EXPECT_TRUE(packet.Valid());
}

TEST(NetworkPacket, ValidRejectsAnOutOfRangeSenderId) {
    ElevatorState state = consistent_state(config::kElevs, 0);

    NetworkPacket packet;
    packet.Init(nullptr, &state, nullptr);

    EXPECT_FALSE(packet.Valid());
}

TEST(NetworkPacket, ValidRejectsACorruptOrderInThePayload) {
    ElevatorState state = consistent_state(0, 0);
    elev::ordersync::OrderTable orders;
    orders.Order(0, 0)->SetAssignedId(99);

    NetworkPacket packet;
    packet.Init(&orders, &state, nullptr);

    EXPECT_FALSE(packet.Valid());
}
