#pragma once

#include <common/types.hpp>

namespace elev::hardware {

void initHardware();

void setMotorDir(elev::common::MotorDir dir);
void setBtnLamp(elev::common::BtnType button, int floor, int value);
void setFloorIndicator(int floor);
void setDoorOpenLamp(int value);
void setStopLamp(int value);

int getBtnSignal(elev::common::BtnType btn, int floor);
int getFloorSensor(void);
int getStopSignal(void);
int getObs(void);

}


