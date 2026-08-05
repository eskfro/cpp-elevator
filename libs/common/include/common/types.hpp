#pragma once

#include <cstdint>
#include <string>
#include <iostream>

// Libs
#include <common/config.hpp>

namespace elev::common {

#define BETWEEN_FLOORS -1

using ButtonFlags = std::array<bool, elev::config::N_BUTTONS>;


enum class DoorState : bool {
    CLOSED = false,
    OPEN = true,   
};


enum class Inertia : std::uint8_t {
    UP,
    DOWN, 
    NONE,
};


enum class MotorDir : std::int8_t {
    DOWN = -1,
    STOP = 0,
    UP = 1,
    ERR = 2,
};


enum class MovingState : std::uint8_t {
    IDLE, 
    DOOR_OPEN,
    MOVING,
    ERR,
};


struct DirMovPair {
    MotorDir motor_dir;
    MovingState moving_state;
};


enum class BtnType : std::uint8_t {
    HALL_UP = 0,
    HALL_DOWN = 1,
    CAB = 2,
};


enum class OrderStatus : std::uint8_t {
    NONE = 0,
    REQUESTED = 1,
    CONFIRMED = 2,
    CLEAR = 3,
};




inline std::string btnTypeToStr(BtnType btn) {
    switch (btn) {
        case BtnType::HALL_UP:   return "HALL_UP";
        case BtnType::HALL_DOWN: return "HALL_DOWN";
        case BtnType::CAB:       return "CAB";
        default:                 return "UNKNOWN_BTN";
    }
}


inline void Print(std::string s) {
    std::cout << s << std::endl;
}


inline void PrintBtnPress(int elevID, int floor, BtnType btn) {
    std::cout << "[ Elevator " << elevID << " ] - buttonpress " << btnTypeToStr(btn) << \
    " at floor " << floor << std::endl;
}

inline void PrintError(std::string_view msg) {
    // Prints message in red
    std::cerr << "\033[31m" << msg << "\033[0m\n";
}


}// namespace elev::common
