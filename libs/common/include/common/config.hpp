#pragma once

namespace elev::config {

    // Hardware config
    constexpr char kIpHw[] = "localhost";
    constexpr char kPortHw[] = "15657";
    constexpr char kConfigPath[] = "/home/eskfro/cpp-elevator/libs/hardware/config/elevator_hardware.con";
    constexpr int kBetweenFloors = -1;

    // Elevator config
    constexpr int kElevs = 4;
    constexpr int kFloors = 4;
    constexpr int kButtons = 3;
    constexpr int kDoorOpenTimeMs = 3000;
    constexpr int kReassignOrderTimeMs = 15000;

} // namespace elev::config
