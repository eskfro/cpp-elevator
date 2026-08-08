#pragma once

// TODO: code to bcast orders and elev state

#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"
#include <netinet/in.h>

namespace elev::network {

// Network to be broadcasted using UDP
#pragma pack(push, 1)
class NetworkPacket {
    public:
        NetworkPacket() = default;

        void Init(const elev::ordersync::OrderMatrix* matrix, const elev::elevator::ElevatorState* state);    

        void SetID(int id) { id_ = id; }
    
        int ID() { return id_; }
        elev::ordersync::OrderMatrix* Orders() { return &orders_; } 
        elev::elevator::ElevatorState* State() { return &state_; }

    private:
        int id_;
        elev::ordersync::OrderMatrix orders_;
        elev::elevator::ElevatorState state_;

};
#pragma pack(pop)

class UdpBroadcaster {
    public:
        UdpBroadcaster(uint16_t port, const std::string& bcast_ip);
        ~UdpBroadcaster();

        UdpBroadcaster(const UdpBroadcaster&) = delete;
        UdpBroadcaster& operator=(const UdpBroadcaster&) = delete;

        bool SendPacket(const NetworkPacket* packet);

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

} // namespace elev::network