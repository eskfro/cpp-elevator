add_executable(elevator-node 
    src/main.cpp 
    src/elevator_node.cpp
)

target_include_directories(elevator-node PRIVATE include)

target_link_libraries(elevator-node PRIVATE common)
target_link_libraries(elevator-node PRIVATE hardware)
target_link_libraries(elevator-node PRIVATE control)
target_link_libraries(elevator-node PRIVATE network)
target_link_libraries(elevator-node PRIVATE ordersync)
target_link_libraries(elevator-node PRIVATE elevator)