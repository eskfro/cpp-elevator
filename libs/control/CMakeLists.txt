add_library(control
    src/fsm.cpp
    src/requests.cpp
)
target_include_directories(control PUBLIC include)
target_link_libraries(control PUBLIC common)
