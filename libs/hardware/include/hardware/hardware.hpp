#pragma once

#include <common/types.hpp>

namespace elev::hardware {

void init_hardware(int id, int sim);

void set_motor_dir(elev::common::MotorDir dir);
void set_btn_lamp(elev::common::BtnType button, int floor, int value);
void set_floor_indicator(int floor);
void set_door_open_lamp(int value);
void set_stop_lamp(int value);

int get_button_signal(elev::common::BtnType btn, int floor);
int get_floor_sensor(void);
int get_stop_signal(void);
int get_obstruction_signal(void);

}


