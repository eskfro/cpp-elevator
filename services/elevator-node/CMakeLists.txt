add_executable(elevator-node src/main.cpp)
target_link_libraries(elevator-node PRIVATE 
    common
    hardware
    control
    network
    ordercontrol
    elevator
)
