#pragma once

#include <cstdint>
#include <string>

// Libs
#include <common/config.hpp>

namespace elev::common {

#define BETWEEN_FLOORS -1


using ButtonFlags = std::array<bool, elev::config::N_BUTTONS>;


enum class MotorDir : std::int8_t {
    DOWN = -1,
    STOP = 0,
    UP = 1,
    ERR = 2,
};


enum class Movement : std::uint8_t {
    IDLE, 
    DOOR_OPEN,
    MOVING,
    ERR,
};


struct DirMovPair {
    MotorDir dir;
    Movement mov;
};


enum class BtnType : std::uint8_t {
    HALL_UP = 0,
    HALL_DOWN = 1,
    CAB = 2,
};


enum class OrderStatus : std::uint8_t {
    NONE,
    REQUESTED,
    CONFIRMED,
    CLEAR,
};


struct ElevatorState {
    int ID;
    int floor;
    MotorDir dir;
    Movement mov;
    bool active;
    bool obstruction;
    bool door_open;
    std::string IP;
};



}
