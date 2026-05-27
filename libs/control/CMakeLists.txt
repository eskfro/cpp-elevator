add_library(control
    src/controller.cpp
    src/requests.cpp
)
target_include_directories(control PUBLIC include)

target_link_libraries(control PUBLIC common)
target_link_libraries(control PRIVATE elevator)
target_link_libraries(control PUBLIC ordersync)