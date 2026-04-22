#include "globals.h"
#include "../include/task_health.h"
#include "../include/battery.h"
#include <SPIFFS.h>
#include <string.h>

uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE] = {0};
bool systemOK = true;

namespace {
struct TelemetryFrame {
    uint32_t uptimeMs;
    float temperatureC;
    float batteryPercent;
    uint8_t status;
    uint8_t crc;
};

SatMode_t modeFromTemperature(float temperatureC) {
    if (temperatureC > TEMP_SAFE_HIGH_THRESHOLD || temperatureC < TEMP_SAFE_LOW_THRESHOLD) {
        return MODE_SAFE;
    }
    if (temperatureC >= TEMP_COOLING_THRESHOLD) {
        return MODE_COOLING;
    }
    return MODE_NOMINAL;
}

SatMode_t modeFromBattery(float batteryPercent) {
    if (batteryPercent < BATTERY_SAFE_THRESHOLD) {
        return MODE_SAFE;
    }
    if (batteryPercent <= BATTERY_NOMINAL_THRESHOLD) {
        return MODE_LOW_POWER;
    }
    return MODE_NOMINAL;
}

SatMode_t chooseMostRestrictiveMode(SatMode_t batteryMode, SatMode_t temperatureMode) {
    return (static_cast<int>(batteryMode) >= static_cast<int>(temperatureMode)) ? batteryMode : temperatureMode;
}
} // namespace

float readInternalTemp(void) {
    return static_cast<float>(readManualTemperature());
}

float readBatteryLevel(void) {
    return static_cast<float>(simulateBatteryLevel());
}


void vTaskHealth(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Frecuencia dinámica: nominal cada 2s, anomalía 500ms
    TickType_t xFrequency = pdMS_TO_TICKS(2000);

    for (;;) {
        // 1. RECOPILACIÓN (telemetría externa e interna)
        // se lee toda la información disponible (temperatura, batería, paneles, etc)
        processManualInputFromSerial();
        float currentTemp = readInternalTemp();
        float currentBattery = readBatteryLevel();

        // 2. CHECKING (generación de estado y modo propuesto por bandas)
        const SatMode_t batteryMode = modeFromBattery(currentBattery);
        const SatMode_t temperatureMode = modeFromTemperature(currentTemp);
        const SatMode_t proposedMode = chooseMostRestrictiveMode(batteryMode, temperatureMode);

        healthProposedMode = proposedMode;

        if (proposedMode == MODE_SAFE) {
            systemOK = false;
            healthStatus = HEALTH_NOK;

            if (batteryMode == MODE_SAFE) {
                healthError = LOW_BATTERY_ERROR;
            } else if (currentTemp > TEMP_SAFE_HIGH_THRESHOLD) {
                healthError = OVERHEAT_ERROR;
            } else {
                healthError = UNKNOWN_ERROR;
            }

            Serial.printf("[HEALTH] NOK | Modo propuesto: SAFE | Bat: %.1f%% | Temp: %.1f C\n", currentBattery, currentTemp);
            xFrequency = pdMS_TO_TICKS(500);
        } else {
            systemOK = true;
            healthStatus = HEALTH_OK;
            healthError = NO_ERROR;
            Serial.printf("[HEALTH] OK | Modo actual: %s | Modo propuesto: %s | Bat: %.1f%% | Temp: %.1f C\n",
                          modeToString(currentMode),
                          modeToString(proposedMode),
                          currentBattery,
                          currentTemp);
            xFrequency = pdMS_TO_TICKS(2000);
        }

        // 4. STATUS REPORT (empaquetado y cifrado)
        // Guardar valores globales para la tarea de Downlink
        lastTemperature = currentTemp;
        lastBattery = currentBattery;
        systemUptime = millis();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}