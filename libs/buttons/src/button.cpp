#include "buttons/button.hpp"
#include "common/config.hpp"
#include "common/types.hpp"
#include "hardware/hardware.hpp"
#include <cstddef>

elev::button::Button::Button(int floor, elev::common::BtnType btn) : 
    curr_press_(false), 
    prev_press_(false),
    btn_(btn),
    floor_(floor) {};

bool elev::button::Button::Pressed() {
    bool button_pressed = false;
    curr_press_ = elev::hardware::get_button_signal(btn_, floor_);

    if (curr_press_ == true && prev_press_ == false) {
        button_pressed = true;
    }

    prev_press_ = curr_press_;

    return button_pressed;
}


void elev::button::Button::ConfigureButton(int floor, elev::common::BtnType btn) {
    floor_ = floor;
    btn_ = btn;
}


elev::buttons::ButtonMatrix::ButtonMatrix() {
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            matrix_[f][b].ConfigureButton(f, (elev::common::BtnType)b);
        }
    }
}


elev::button::Button* elev::buttons::ButtonMatrix::Button(int floor, elev::common::BtnType btn) {
    return &matrix_[floor][static_cast<std::size_t>(btn)];
}







