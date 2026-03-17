#include <HardwareSerial.h>
#include "task_monitor.h"
#include "globals.h"
#include "../include/task_monitor.h"

void vTaskMonitor(void *pvParameters) {
    for (;;) {
        Serial.println("\n--- MONITOR DE SISTEMA ---");

        if (hMonitor) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(hMonitor);
            Serial.printf("Tarea: Monitor | Stack minimo historico libre: %u bytes\n", hwm);
        }
        if (hChecking) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(hChecking);
            Serial.printf("Tarea: Checking | Stack minimo historico libre: %u bytes\n", hwm);
            if (hwm < 50) Serial.printf("ALERTA: Stack critico en Checking\n");
        }
        if (hModeManager) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(hModeManager);
            Serial.printf("Tarea: ModeManager | Stack minimo historico libre: %u bytes\n", hwm);
            if (hwm < 50) Serial.printf("ALERTA: Stack critico en ModeManager\n");
        }
        if (ledsPulsingHandle) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(ledsPulsingHandle);
            Serial.printf("Tarea: LedsPulsing | Stack minimo historico libre: %u bytes\n", hwm);
            if (hwm < 50) Serial.printf("ALERTA: Stack critico en LedsPulsing\n");
        }
        if (hPostDeploy) {
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(hPostDeploy);
            Serial.printf("Tarea: PostDeploy | Stack minimo historico libre: %u bytes\n", hwm);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}