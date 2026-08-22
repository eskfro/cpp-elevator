add_library(network
    src/udp_bcast.cpp
    src/peers.cpp
    src/order_timers.cpp
)

target_include_directories(network PUBLIC include)

target_link_directories(network PUBLIC common)

target_link_libraries(network PUBLIC ordersync)
target_link_libraries(network PUBLIC elevator)