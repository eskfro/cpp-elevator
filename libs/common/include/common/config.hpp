#pragma once

namespace elev::config {

// Hardware config
constexpr char kIpHw[] = "localhost";
constexpr char kPortHw[] = "15657";
constexpr char kConfigPath[] = "/home/eskfro/cpp-elevator/libs/hardware/config/elevator_hardware.con";
constexpr int kBetweenFloors = -1;

// Building
constexpr int kElevs = 4;
constexpr int kFloors = 4;
constexpr int kButtons = 3;

// Time
constexpr int kDoorOpenTimeMs = 3000;
constexpr int kReassignOrderTimeMs = 20000;
constexpr int kWatchdogTimeMs = 1000;

}  // namespace elev::config
