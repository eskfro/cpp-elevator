add_library(buttons
    src/button.cpp
)

target_include_directories(buttons PUBLIC include)

target_link_libraries(buttons PUBLIC common)
target_link_libraries(buttons PUBLIC hardware)