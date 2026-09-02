# cpp-elevator
### About
- TTK4145 distributed elevator semester project
- I have already done the project using the Go-language and i also wanted to try it in C++
- The goal is learning modern C++ development and write good code
### Network design
- p2p UDP bcast of ElevatorState and OrderTable
### Build-system
- CMake
### Building elevator-node
``` 
mkdir build
cmake --build build -j2
```
### Build elevator server
```bash
dmd -w -g src/sim_server.d src/timer_event.d -ofSimElevatorServer
```
### Run elevator node and server (id = 1-3)
```bash
make sim(1-3)
```
### Run all elevator-node's and servers (id = 1,2,3)
```bash
make simall
```  
### Run tests
```bash
ctest --test-dir build
```
### Simulator environment
<img width="1920" height="1031" alt="image" src="https://github.com/user-attachments/assets/84894df0-8a9f-41f0-8e35-a28f7627d68c" />

