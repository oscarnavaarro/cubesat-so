#include "globals.h"
#include "../include/task_mode_manager.h"
#include "../include/battery.h"

void vTaskModeManager(void *pvParameters) {
    for (;;) {
        processManualInputFromSerial();

        int battery_level = simulateBatteryLevel();

        // si se detecta batería baja, cambia a LOW_POWER
        if (currentMode == MODE_NOMINAL && battery_level < 20) {
            currentMode = MODE_LOW_POWER;
            Serial.printf("[MODES] Cambio a LOW POWER: Suspendiendo experimentos...\n");
            if (ledsPulsingHandle) {
                vTaskSuspend(ledsPulsingHandle);
            }
        }

        // si la bateria se recupera, vuelve a modo nominal y reactiva el payload
        if (currentMode == MODE_LOW_POWER && battery_level > 35) {
            currentMode = MODE_NOMINAL;
            Serial.printf("[MODES] Bateria recuperada: Volviendo a NOMINAL.\n");
            if (ledsPulsingHandle) {
                vTaskResume(ledsPulsingHandle);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}