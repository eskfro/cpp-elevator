add_library(elevator
    src/elevator.cpp
)
target_include_directories(elevator PUBLIC include)

target_link_libraries(elevator PUBLIC common)
target_link_libraries(elevator PRIVATE hardware)


