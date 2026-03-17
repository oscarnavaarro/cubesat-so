#include "globals.h"
#include "modes.h"
#include "../include/task_health.h"

void vTaskHealth(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Frecuencia dinámica: nominal cada 2s, anomalía 500ms
    TickType_t xFrequency = pdMS_TO_TICKS(2000);

    for (;;) {
        // 1. RECOPILACIÓN (telemetría externa e interna)
        // se lee toda la información disponible (temperatura, batería, paneles, etc)
        float currentTemp = readInternalTemp();
        float currentBattery = readBatteryLevel();

        // 2. CHECKING (generación de OK/NOK)
        // el gestor de salud trabaja con toda la información disponible
        if (currentTemp > MAX_TEMP_THRESHOLD || currentBattery < MIN_BATTERY_THRESHOLD) {
            systemOK = false;
            Serial.printf("[HEALTH] Estado: NOK detectado!\n");

            // 3. LLAMADA A SOLVER (acción inmediata)
            vSolver(currentTemp > MAX_TEMP_THRESHOLD ? OVERHEAT_ERROR : LOW_BATTERY_ERROR);
            // en este ejemplo, el solver simplemente imprime un mensaje, pero podría ser una acción más compleja 
            // como activar un sistema de enfriamiento o reducir la carga del sistema.

            xFrequency = pdMS_TO_TICKS(500); // aumentar frecuencia en caso de anomalía
        } else {
            systemOK = true;
            xFrequency = pdMS_TO_TICKS(2000); // frecuencia nominal
        }

        // 4. STATUS REPORT (empaquetado y cifrado)
        // se genera el reporte completo y se deja en el buffer para Downlink
        packAndEncryptTelemetry(currentTemp, currentBattery, systemOK);
        Serial.printf("[HEALTH] Telemetría generada y cifrada en buffer.\n");

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}