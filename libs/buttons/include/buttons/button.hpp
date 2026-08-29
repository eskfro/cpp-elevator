#pragma once

#include <array>
#include <hardware/hardware.hpp>

#include "common/config.hpp"
#include "common/types.hpp"

using namespace elev::config;

namespace elev::buttons {

class Button {
public:
    Button() = default;
    Button(int floor, elev::common::BtnType btn);
    bool Pressed();
    void Init(int floor, elev::common::BtnType btn);

private:
    int floor_{};
    elev::common::BtnType btn_{};
    bool curr_press_{};
    bool prev_press_{};
};

class ButtonTable {
public:
    ButtonTable();
    elev::buttons::Button* Button(int floor, elev::common::BtnType btn);

private:
    std::array<std::array<elev::buttons::Button, kButtons>, kFloors> matrix_{};
};

class StopButton {
public:
    StopButton() = default;
    bool Pressed();

private:
    bool curr_press_{};
    bool prev_press_{};
};

}  // namespace elev::buttons
