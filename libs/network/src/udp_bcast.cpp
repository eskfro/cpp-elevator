#include "network/udp_bcast.hpp"

#include <asm-generic/socket.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"

namespace elev::network {

void NetworkPacket::Init(const elev::ordersync::OrderMatrix* matrix,
                        const elev::elevator::ElevatorState* state) {
    if (matrix) orders_ = *matrix;
    if (state) state_ = *state;
}

UdpBroadcaster::UdpBroadcaster(uint16_t port, const std::string& bcast_ip) {
    // Create udp socket (SOCK_DGRAM)
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "[UDP] Error creating socket: " << strerror(errno) << std::endl;
        return;
    }

    // Enable SO_BROADCAST
    int bcast_enable = 1;
    if ((setsockopt(socket_fd_, SOL_SOCKET, SO_BROADCAST, &bcast_enable, sizeof(bcast_enable)) < 0)) {
        std::cerr << "[UDP] Error setting SO_BROADCAST: " << strerror(errno) << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    // Configure target bcast adress
    std::memset(&bcast_addr_, 0, sizeof(bcast_addr_));
    bcast_addr_.sin_family = AF_INET;
    bcast_addr_.sin_port = htons(port);

    if (inet_pton(AF_INET, bcast_ip.c_str(), &bcast_addr_.sin_addr) <= 0) {
        std::cerr << "[UDP] Invalid broadcast IP adress: " << bcast_ip << std::endl;
    }
}


UdpBroadcaster::~UdpBroadcaster() {
    if (socket_fd_ > 0) {
        close(socket_fd_);
    }
}


bool UdpBroadcaster::SendPacket(const NetworkPacket* packet) {
    if (socket_fd_ < 0 || !packet) {
        return false;
    }

    // Send bytes
    ssize_t bytes_sent = sendto(socket_fd_, packet, sizeof(NetworkPacket), 0
        ,reinterpret_cast<const struct sockaddr*>(&bcast_addr_), sizeof(bcast_addr_));

    if (bytes_sent < 0) {
        std::cerr << "[UDP] Broadcast send failed: " << strerror(errno) << std::endl;
        return false;
    }


    return static_cast<size_t>(bytes_sent) == sizeof(NetworkPacket);

}



} // namespace elev::network