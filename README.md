# cpp-elevator
### About
- TTK4145 Distributed Elevator Semester Project
- I have already done the project using the Go-language. I now want to do the project in C++
- The goal is learning modern C++ development and write good code
### Network design
- p2p UDP bcast of ElevatorState and OrderTable
### Build system
- CMake
### Build and run elevator node
```bash
make node
```
### Build elevator server
```bash
dmd -w -g src/sim_server.d src/timer_event.d -ofSimElevatorServer
```
### Start elevator server
```bash
make server
```
### Run tests
```bash
ctest --test-dir build
```
### Simulator environment image
<img width="1920" height="1031" alt="image" src="https://github.com/user-attachments/assets/84894df0-8a9f-41f0-8e35-a28f7627d68c" />

