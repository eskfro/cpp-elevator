#include <assert.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/types.hpp"
#include "common/utils.hpp"
#include <common/config.hpp>
#include <hardware/hardware.hpp>

namespace elev::hardware {

static int sockfd;
static pthread_mutex_t sockmtx;

void init_hardware(int id, int sim) {
    int ok;

    char ip[16];
    char port[8];
    strncpy(ip, elev::config::kIpHw, sizeof(ip) - 1);
    strncpy(port, elev::config::kPortHw, sizeof(port) - 1);

    if (sim == 1) {
        int base = atoi(port);
        int computed = base + id;
        snprintf(port, sizeof(port), "%d", computed);
    }

    pthread_mutex_init(&sockmtx, NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) elev::common::Abort("[HW] Unable to set up socket");

   struct addrinfo hints = {
        .ai_flags = 0,
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
        .ai_addrlen = 0,
        .ai_addr = nullptr,
        .ai_canonname = nullptr,
        .ai_next = nullptr,
    };

    struct addrinfo* res;

    ok = getaddrinfo(ip, port, &hints, &res);
    if (ok != 0) elev::common::Abort("[HW] getaddrinfo failed");

    ok = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (ok != 0) elev::common::Abort("[HW] connection failed");

    freeaddrinfo(res);

    char msg[4] = {0};
    send(sockfd, msg, 4, 0);
}

void set_motor_dir(elev::common::MotorDir dir) {
    if (dir == common::MotorDir::Err) elev::common::Abort("[HW] MotorDir Err has no defined actuator");

    int dirn = static_cast<int>(dir);

    char msg[4] = {1, static_cast<char>(dirn)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}

void set_btn_lamp(elev::common::BtnType btn, int floor, int value) {
    int button = static_cast<int>(btn);

    assert(floor >= 0);
    assert(floor < config::kFloors);
    assert(button >= 0);
    assert(button < config::kButtons);

    char msg[4] = {2, static_cast<char>(button), static_cast<char>(floor), static_cast<char>(value)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}

void set_floor_indicator(int floor) {
    assert(floor >= 0);
    assert(floor < elev::config::kFloors);

    char msg[4] = {3, static_cast<char>(floor)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}

void set_door_open_lamp(int value) {
    char msg[4] = {4, static_cast<char>(value)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}

void set_stop_lamp(int value) {
    char msg[4] = {5, static_cast<char>(value)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}

int get_button_signal(elev::common::BtnType btn, int floor) {
    int button = static_cast<int>(btn);

    char msg[4] = {6, static_cast<char>(button), static_cast<char>(floor)};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}

int get_floor_sensor(void) {
    char msg[4] = {7};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1] ? buf[2] : -1;
}

int get_stop_signal(void) {
    char msg[4] = {8};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}

int get_obstruction_signal(void) {
    char msg[4] = {9};
    pthread_mutex_lock(&sockmtx);
    send(sockfd, msg, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}

void shutdown_hardware() {
    close(sockfd);
    pthread_mutex_destroy(&sockmtx);
}

}  // namespace elev::hardware