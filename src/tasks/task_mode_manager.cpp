#include "globals.h"
#include "../include/task_mode_manager.h"

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