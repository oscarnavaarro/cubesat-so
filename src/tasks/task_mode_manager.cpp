#include "globals.h"
#include "../include/task_mode_manager.h"
#include "../include/battery.h"

namespace {
SatMode_t modeFromTemperature(int temperatureC) {
    if (temperatureC > 60 || temperatureC < -20) {
        return MODE_SAFE;
    }
    if (temperatureC >= 45) {
        return MODE_COOLING;
    }
    return MODE_NOMINAL;
}

SatMode_t modeFromBattery(int batteryLevel) {
    if (batteryLevel < 20) {
        return MODE_SAFE;
    }
    if (batteryLevel <= 50) {
        return MODE_LOW_POWER;
    }
    return MODE_NOMINAL;
}

SatMode_t chooseMostRestrictiveMode(SatMode_t batteryMode, SatMode_t temperatureMode) {
    if (batteryMode == MODE_SAFE || temperatureMode == MODE_SAFE) {
        return MODE_SAFE;
    }
    if (temperatureMode == MODE_COOLING) {
        return MODE_COOLING;
    }
    if (batteryMode == MODE_LOW_POWER) {
        return MODE_LOW_POWER;
    }
    return MODE_NOMINAL;
}

const char *modeToString(SatMode_t mode) {
    switch (mode) {
        case MODE_NOMINAL:
            return "NOMINAL";
        case MODE_LOW_POWER:
            return "LOW_POWER";
        case MODE_SAFE:
            return "SAFE";
        case MODE_COOLING:
            return "COOLING";
        default:
            return "UNKNOWN";
    }
}
} // namespace

void vTaskModeManager(void *pvParameters) {
    bool payloadIsSuspended = false;

    for (;;) {
        processManualInputFromSerial();

        int battery_level = simulateBatteryLevel();
        int temperature_c = readManualTemperature();

        SatMode_t nextMode = chooseMostRestrictiveMode(modeFromBattery(battery_level), modeFromTemperature(temperature_c));

        if (nextMode != currentMode) {
            SatMode_t previousMode = currentMode;
            currentMode = nextMode;

            Serial.printf("[MODES] %s -> %s | Bat: %d%% | Temp: %d C\n",
                          modeToString(previousMode),
                          modeToString(currentMode),
                          battery_level,
                          temperature_c);
        }

        const bool shouldSuspendPayload = (currentMode != MODE_NOMINAL);
        if (ledsPulsingHandle && shouldSuspendPayload != payloadIsSuspended) {
            if (shouldSuspendPayload) {
                vTaskSuspend(ledsPulsingHandle);
            } else {
                vTaskResume(ledsPulsingHandle);
            }
            payloadIsSuspended = shouldSuspendPayload;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}