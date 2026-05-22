add_library(movement
    src/fsm.cpp
    src/movement.cpp
    src/requests.cpp
)
target_include_directories(movement PUBLIC include)
target_link_libraries(movement PUBLIC common)
