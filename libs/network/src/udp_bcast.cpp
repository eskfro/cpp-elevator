#include "network/udp_bcast.hpp"

#include <asm-generic/socket.h>
#include <cstddef>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/config.hpp"
#include "elevator/elevator_state.hpp"
#include "ordersync/ordersync.hpp"

namespace elev::network {

void NetworkPacket::Init(elev::ordersync::OrderTable* orders,
                        elev::elevator::ElevatorState* state,
                        std::array<std::array<elev::ordersync::Order, kFloors>, kElevs>* cab_button_orders) {
    if (orders) orders_ = *orders;
    if (state) {
        id_ = state->Id();
        state_ = *state;
    }
    if (cab_button_orders) cab_button_orders_ = *cab_button_orders;
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

bool UdpBroadcaster::SendPacket(NetworkPacket* packet) {

    if (packet->ID() < 0 || packet->ID() >= kElevs) return false;

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

UdpReciever::UdpReciever(uint16_t port) {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "[UDP RX] Error creating socket: " << strerror(errno) << std::endl;
        return;
    }

    int reuse = 1;
    setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in rx_addr{};
    std::memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family = AF_INET;
    rx_addr.sin_port = htons(port);
    rx_addr.sin_addr.s_addr = INADDR_ANY; // all network interfaces

    if (bind(socket_fd_, reinterpret_cast<struct sockaddr*>(&rx_addr), sizeof(rx_addr)) < 0) {
        std::cerr << "[UDP Rx] Error binding socket to port " << port << ": " << strerror(errno) << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

UdpReciever::~UdpReciever() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
    }
}

bool UdpReciever::RecievePacket(NetworkPacket* packet) {
    if (socket_fd_ < 0 || !packet) return false;

    struct sockaddr_in src_addr{};
    socklen_t addr_len = sizeof(src_addr);

    ssize_t bytes_rcvd = recvfrom(socket_fd_, packet, sizeof(NetworkPacket),
        0, reinterpret_cast<struct sockaddr*>(&src_addr), &addr_len);
    
    if (bytes_rcvd < 0 || static_cast<size_t>(bytes_rcvd) != sizeof(NetworkPacket)) {
        return false;
    }
    if (packet->ID() < 0 || packet->ID() >= kElevs) return false;

    return true;
}

} // namespace elev::network