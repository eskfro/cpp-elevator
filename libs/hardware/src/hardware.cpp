#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include <hardware/hardware.hpp>
#include <hardware/con_load.hpp>
#include <common/config.hpp>

namespace elev::hardware {


static int sockfd;
static pthread_mutex_t sockmtx;

void init_hardware() {
    char ip[16]; 
    char port[8]; 

    strncpy(ip, elev::config::IP_HW, sizeof(ip)-1);
    strncpy(port, elev::config::PORT_HW, sizeof(port)-1);

    con_load(elev::config::CONFIG_FILE,
        con_val("com_ip",   ip,   "%s")
        con_val("com_port", port, "%s")
    )
    
    pthread_mutex_init(&sockmtx, NULL);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1 && "Unable to set up socket");
    
    struct addrinfo hints = {
        .ai_family      = AF_INET, 
        .ai_socktype    = SOCK_STREAM, 
        .ai_protocol    = IPPROTO_TCP,
    };
    struct addrinfo* res;
    getaddrinfo(ip, port, &hints, &res);
    
    int fail = connect(sockfd, res->ai_addr, res->ai_addrlen);
    assert(fail == 0 && "Unable to connect to simulator server");
    
    freeaddrinfo(res);
    
    send(sockfd, (char[4]) {0}, 4, 0);
}


void set_motor_dir(elev::common::MotorDir dir) {

    int dirn = static_cast<int>(dir);

    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {1, dirn}, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}


void set_btn_lamp(elev::common::BtnType btn, int floor, int value) {

    int button = static_cast<int>(btn);

    assert(floor >= 0);
    assert(floor < elev::config::N_FLOORS);
    assert(button >= 0);
    assert(button < elev::config::N_BUTTONS);

    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {2, button, floor, value}, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}


void set_floor_indicator(int floor) {
    assert(floor >= 0);
    assert(floor < elev::config::N_FLOORS);

    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {3, floor}, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}


void set_door_open_lamp(int value) {
    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {4, value}, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}


void set_stop_lamp(int value) {
    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {5, value}, 4, 0);
    pthread_mutex_unlock(&sockmtx);
}


int get_btn_signal(elev::common::BtnType btn, int floor) {

    int button = static_cast<int>(btn);

    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {6, button, floor}, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}


int get_floor_sensor(void) {
    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {7}, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1] ? buf[2] : -1;
}


int get_stop_signal(void) {
    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {8}, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}


int get_obs_signal(void) {
    pthread_mutex_lock(&sockmtx);
    send(sockfd, (char[4]) {9}, 4, 0);
    char buf[4];
    recv(sockfd, buf, 4, 0);
    pthread_mutex_unlock(&sockmtx);
    return buf[1];
}

}