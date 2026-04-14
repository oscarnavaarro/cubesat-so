#ifndef TASK_HEALTH_H
#define TASK_HEALTH_H

#include <stdint.h>

// Definición de buffers para telemetría, simulando almacenamiento en NAND Flash/SDRAM
#define TELEMETRY_BUFFER_SIZE 256

static constexpr float TEMP_SAFE_HIGH_THRESHOLD = 60.0f;
static constexpr float TEMP_COOLING_THRESHOLD = 45.0f;
static constexpr float TEMP_SAFE_LOW_THRESHOLD = -20.0f;

static constexpr float BATTERY_SAFE_THRESHOLD = 20.0f;
static constexpr float BATTERY_NOMINAL_THRESHOLD = 50.0f;

extern uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE];
extern bool systemOK; // OK/NOK

float readInternalTemp(void);
float readBatteryLevel(void);
void packAndEncryptTelemetry(float currentTemp, float currentBattery, bool isSystemOK);

void vTaskHealth(void *pvParameters);

#endif // TASK_HEALTH_H