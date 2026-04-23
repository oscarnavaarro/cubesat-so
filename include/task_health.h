#ifndef TASK_HEALTH_H
#define TASK_HEALTH_H

#include <stdint.h>
#include "sat_config.h"

extern uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE];
extern bool systemOK; // OK/NOK

float readInternalTemp(void);
float readBatteryLevel(void);
void packAndEncryptTelemetry(float currentTemp, float currentBattery, bool isSystemOK);

void vTaskHealth(void *pvParameters);

#endif // TASK_HEALTH_H