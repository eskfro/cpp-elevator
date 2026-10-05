#include "buttons/button.hpp"

#include <cstddef>

#include "common/config.hpp"
#include "common/types.hpp"
#include "hardware/hardware.hpp"

namespace elev::buttons {

OrderButton::OrderButton(int floor, elev::common::BtnType btn) :
    floor_(floor),
    btn_(btn),
    curr_press_(false),
    prev_press_(false) {}

bool OrderButton::Pressed() {
    bool button_pressed = false;
    curr_press_ = elev::hardware::get_button_signal(btn_, floor_);
    if (curr_press_ == true && prev_press_ == false) {
        button_pressed = true;
    }
    prev_press_ = curr_press_;
    return button_pressed;
}

void OrderButton::Init(int floor, elev::common::BtnType btn) {
    floor_ = floor;
    btn_ = btn;
}

OrderButtonTable::OrderButtonTable() {
    for (int f = 0; f < kFloors; f++) {
        for (int b = 0; b < kButtons; b++) {
            matrix_[f][b].Init(f, (elev::common::BtnType)b);
        }
    }
}

OrderButton* OrderButtonTable::Button(
    int floor, elev::common::BtnType btn) {
    return &matrix_[floor][static_cast<std::size_t>(btn)];
}

bool StopButton::Pressed() {
    bool button_pressed = false;
    curr_press_ = elev::hardware::get_stop_signal();
    if (curr_press_ == true && prev_press_ == false) {
        button_pressed = true;
    }
    prev_press_ = curr_press_;
    return button_pressed;
}

} // namespace elev::buttons