# cpp-elevator
### About
- TTK4145 Distributed Elevator Semester Project.
- I have already done the project using the Go-language. I now want to do the project in C++.
- The goal is learning modern C++ development and write good code.
### Network
- p2p UDP bcast of ElevatorState and OrderMatrix for syncing orders
### Build System
- CMake
### Build and Run Elevator Node
```bash
make node
```
### Build Elevator Server
```bash
dmd -w -g src/sim_server.d src/timer_event.d -ofSimElevatorServer
```
### Start Elevator Server
```bash
make server
```
