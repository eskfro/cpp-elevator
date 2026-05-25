add_library(ordersync
    src/assigner.cpp
    src/ordersync.cpp
)
target_include_directories(ordersync PUBLIC include)
target_link_libraries(ordersync PUBLIC common)