#pragma once

// TODO: code to bcast orders and elev state

#include <netinet/in.h>

#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"

namespace elev::network {

// Network to be broadcasted using UDP
#pragma pack(push, 1)
class NetworkPacket {
public:
    NetworkPacket() = default;

    void Init(elev::ordersync::OrderTable* orders,
              elev::elevator::ElevatorState* state,
              elev::ordersync::CabOrderTable* cab_button_orders);

    int ID() { return id_; }
    elev::ordersync::OrderTable* Orders() { return &orders_; }
    elev::elevator::ElevatorState* State() { return &state_; }
    elev::ordersync::CabOrderTable* CabButtonOrders() { return &cab_button_orders_; }

private:
    int id_;
    elev::ordersync::OrderTable orders_;
    elev::elevator::ElevatorState state_;
    elev::ordersync::CabOrderTable cab_button_orders_;
};
#pragma pack(pop)

class UdpBroadcaster {
public:
    UdpBroadcaster(uint16_t port, const std::string& bcast_ip);
    ~UdpBroadcaster();

    bool SendPacket(NetworkPacket* packet);

private:
    int socket_fd_{-1};
    struct sockaddr_in bcast_addr_{};
};

class UdpReciever {
public:
    UdpReciever(uint16_t port);
    ~UdpReciever();

    bool RecievePacket(NetworkPacket* packet);

private:
    int socket_fd_{-1};
};

}  // namespace elev::network