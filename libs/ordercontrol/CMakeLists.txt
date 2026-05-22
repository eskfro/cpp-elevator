add_library(ordercontrol
    src/assigner.cpp
    src/ordercontrol.cpp
)
target_include_directories(ordercontrol PUBLIC include)
target_link_libraries(ordercontrol PUBLIC common)