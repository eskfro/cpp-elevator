#pragma once

#include <cstdint>
#include <string>
#include <iostream>

// Libs
#include <common/config.hpp>

namespace elev::common {

using ButtonFlags = std::array<bool, elev::config::kButtons>;

enum class DoorState : bool {
    Closed = false,
    Open = true,   
};

enum class Inertia : std::uint8_t {
    Up,
    Down, 
    None,
};

enum class MotorDir : std::int8_t {
    Down = -1,
    Stop = 0,
    Up = 1,
    Err = 2,
};

enum class MovingState : std::uint8_t {
    Idle, 
    DoorOpen,
    Moving,
    Err,
};

struct DirMovPair {
    MotorDir motor_dir;
    MovingState moving_state;
};

enum class BtnType : std::uint8_t {
    HallUp = 0,
    HallDown = 1,
    Cab = 2,
};

enum class OrderStatus : std::uint8_t {
    None = 0,
    Requested = 1,
    Confirmed = 2,
    Clear = 3,
};

inline std::string BtnTypeToString(BtnType btn) {
    switch (btn) {
        case BtnType::HallUp:   return "HALL_UP";
        case BtnType::HallDown: return "HALL_DOWN";
        case BtnType::Cab:       return "CAB";
        default:                 return "UNKNOWN_BTN";
    }
}

inline void Print(std::string s) {
    std::cout << s << std::endl;
}

inline void PrintBtnPress(int elevID, int floor, BtnType btn) {
    std::cout << "[ Elevator " << elevID << " ] - buttonpress " << BtnTypeToString(btn) << \
    " at floor " << floor << std::endl;
}

inline void PrintError(std::string_view msg) {
    // Prints message in red
    std::cerr << "\033[31m" << msg << "\033[0m\n";
}

}// namespace elev::common
