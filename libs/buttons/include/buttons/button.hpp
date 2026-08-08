#pragma once

#include <array>
#include "common/config.hpp"
#include "common/types.hpp"
#include <hardware/hardware.hpp>

using namespace elev::config;

namespace elev::button {

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

} // namespace elev::button

namespace elev::buttons {

class ButtonTable {
    public:
        ButtonTable();
        elev::button::Button* Button(int floor, elev::common::BtnType btn);
    private:
        std::array<std::array<elev::button::Button, kButtons>, kFloors> matrix_{};
};

} // namespace elev::buttons

