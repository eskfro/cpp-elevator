#include <iostream>

#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>


int main() {

    std::cout << "...main..." << std::endl << elev::config::N_BUTTONS;
    
    elev::hardware::initHardware();


};