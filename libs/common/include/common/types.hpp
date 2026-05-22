#pragma once

#include <cstdint>
#include <string>

using Floor = std::uint8_t;

namespace elevator::common {

enum class Direction : std::uint8_t {
    Up,
    Down,
    Stop,
};

enum class ButtonType : std::uint8_t {
    HallUp,
    HallDown,
    Stop,
};

enum class Movement : std::uint8_t {
    Idle, 
    DoorOpen,
    Moving,
    Err,
};

// [Input]
// Requested order from button press
struct Request {
    Floor floor{};
    ButtonType btn{};
};

// Confirmed order to be distributed to elevID
struct Order {
    int elevID;
    Floor floor{};
    ButtonType btn{};
};



};
