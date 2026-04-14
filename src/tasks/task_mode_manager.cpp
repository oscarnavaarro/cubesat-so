#include "globals.h"
#include "../include/task_mode_manager.h"

namespace {
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
    SatMode_t appliedMode = currentMode;

    for (;;) {
        if (currentMode != appliedMode) {
            Serial.printf("[MODES] %s -> %s\n", modeToString(appliedMode), modeToString(currentMode));
            appliedMode = currentMode;
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