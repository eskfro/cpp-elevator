#pragma once

namespace elev::config {

// Hardware config
constexpr char kIpHw[] = "localhost";
constexpr char kPortHw[] = "15657";
constexpr int kBetweenFloors = -1;

// Building
constexpr int kElevs = 4;
constexpr int kFloors = 4;
constexpr int kButtons = 3;

// Time
constexpr int kDoorOpenTimeMs = 3000;
constexpr int kReassignOrderTimeMs = 25000;
constexpr int kWatchdogTimeMs = 600;
constexpr int kFaultTimeoutMs = 5000;
constexpr int kStartupTimeMs = 2 * kWatchdogTimeMs;

// Cost function penalties
constexpr int kPenaltyDoorOpen = 2;
constexpr int kPenaltyFloorDiff = 3;
constexpr int kPenaltyPerOrder = 5;
constexpr int kPenaltyWrongDir = 10;
constexpr int kPenaltyObstruction = 100;
constexpr int kPenaltyFault = 1000;
constexpr int kPenaltyStopped = 1000;

}  // namespace elev::config
