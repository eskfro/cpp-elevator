add_library(network
    src/udp_bcast.cpp
)
target_include_directories(network PUBLIC include)
target_link_directories(network PUBLIC common)

target_link_libraries(network PUBLIC ordersync)