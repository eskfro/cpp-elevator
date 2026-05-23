#include <iostream>

#include <common/config.hpp>
#include <common/types.hpp>
#include <hardware/hardware.hpp>


int main() {

    elev::hardware::initHardware();

    std::cout << "...main..." << std::endl << elev::config::N_BUTTONS;

};