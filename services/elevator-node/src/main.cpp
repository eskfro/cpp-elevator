#include <iostream>
#include <unistd.h>

#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>
#include <elevator/elevator.hpp>

void runButtonPoller(
    elev::elevator::Elevator& elev
) {
    
};

int main() {

    std::cout << "...main..." << std::endl << elev::config::N_BUTTONS;
    
    elev::hardware::initHardware();

    while (true) { sleep(1); };


};