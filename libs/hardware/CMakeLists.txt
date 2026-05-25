add_library(hardware
    src/hardware.cpp
)
target_include_directories(hardware PUBLIC include)

target_link_libraries(hardware PUBLIC common)
