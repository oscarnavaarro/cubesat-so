// battery.h
#ifndef BATTERY_H
#define BATTERY_H

int simulateBatteryLevel(void);
int readManualTemperature(void);
void adjustBatteryByStep(int delta);
void adjustTemperatureByStep(int delta);
void processManualInputFromSerial(void);

#endif // BATTERY_H