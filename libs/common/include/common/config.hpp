#pragma once

#include <string>

namespace elev::config {

    // Hardware config
    constexpr char IP_HW[] = "localhost";
    constexpr char PORT_HW[] = "15657";
    constexpr char CONFIG_FILE[] = "/home/eskfro/cpp-elevator/libs/hardware/config/elevator_hardware.con";

    // Elevator config
    constexpr int N_ELEVS = 4;
    constexpr int N_FLOORS = 4;
    constexpr int N_BUTTONS = 3;

    constexpr int DOOR_OPEN_TIME_MS = 3000;

}
