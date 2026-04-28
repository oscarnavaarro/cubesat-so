// FDIR - Fault Detection, Isolation and Recovery
// Esta tarea es el subsistema de detección de fallos del satélite.
// Su ÚNICA responsabilidad es leer sensores, actualizar los valores en telemetría,
// y emitir eventos hacia la cola FDIR.
// NO decide modos de operación ni estados de salud globales.

#include "globals.h"
#include "../include/task_health.h"
#include "../include/battery.h"
#include <string.h>

uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE] = {0};

float readInternalTemp(void) {
    return static_cast<float>(readManualTemperature());
}

float readBatteryLevel(void) {
    return static_cast<float>(simulateBatteryLevel());
}

// Clasifica la temperatura en un evento FDIR.
static FdirEvent_t classifyTemperature(float temperatureC) {
    if (temperatureC > TEMP_SAFE_HIGH_THRESHOLD) {
        return FDIR_EVENT_OVERHEAT;
    }
    if (temperatureC < TEMP_SAFE_LOW_THRESHOLD) {
        return FDIR_EVENT_UNDERHEAT;
    }
    if (temperatureC >= TEMP_COOLING_THRESHOLD) {
        return FDIR_EVENT_HIGH_TEMP;
    }
    return FDIR_EVENT_NOMINAL;
}

// Clasifica el nivel de batería en un evento FDIR.
static FdirEvent_t classifyBattery(float batteryPercent) {
    if (batteryPercent < BATTERY_SAFE_THRESHOLD) {
        return FDIR_EVENT_LOW_BATTERY;
    }
    if (batteryPercent <= BATTERY_NOMINAL_THRESHOLD) {
        return FDIR_EVENT_LOW_POWER_BATTERY;
    }
    return FDIR_EVENT_NOMINAL;
}

// Devuelve el evento más grave de los dos recibidos.
// El enum está ordenado por severidad (NOMINAL=0, ..., OVERHEAT/UNDERHEAT).
static FdirEvent_t mostSevereEvent(FdirEvent_t a, FdirEvent_t b) {
    return (static_cast<int>(a) >= static_cast<int>(b)) ? a : b;
}

void vTaskHealth(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xFrequency = pdMS_TO_TICKS(DELAY_HEALTH_NOMINAL_MS);

    for (;;) {
        // 1. RECOPILACIÓN: leer sensores
        processManualInputFromSerial();
        const float currentTemp    = readInternalTemp();
        const float currentBattery = readBatteryLevel();

        // 2. CLASIFICACIÓN: convertir lecturas en eventos FDIR
        const FdirEvent_t tempEvent    = classifyTemperature(currentTemp);
        const FdirEvent_t batteryEvent = classifyBattery(currentBattery);
        const FdirEvent_t worstEvent   = mostSevereEvent(tempEvent, batteryEvent);

        // 3. EMISIÓN: enviar el evento más grave al Mode Manager.
        if (fdirQueue != NULL) {
            xQueueSend(fdirQueue, &worstEvent, 0);
        }

        // 4. FRECUENCIA DE REPORTE: Sólo aceleramos a 500ms si el evento requiere modo SAFE
        bool isSafeEvent = (worstEvent == FDIR_EVENT_OVERHEAT) || 
                           (worstEvent == FDIR_EVENT_UNDERHEAT) || 
                           (worstEvent == FDIR_EVENT_LOW_BATTERY);
                           
        if (isSafeEvent) {
            xFrequency = pdMS_TO_TICKS(DELAY_HEALTH_ALERT_MS);
            Serial.printf("[FDIR] ALERTA CRITICA (%d) | Bat: %.1f%% | Temp: %.1f C\n",
                          static_cast<int>(worstEvent), currentBattery, currentTemp);
        } else if (worstEvent != FDIR_EVENT_NOMINAL) {
            xFrequency = pdMS_TO_TICKS(DELAY_HEALTH_NOMINAL_MS);
            Serial.printf("[FDIR] EVENTO (%d) | Bat: %.1f%% | Temp: %.1f C\n",
                          static_cast<int>(worstEvent), currentBattery, currentTemp);
        } else {
            xFrequency = pdMS_TO_TICKS(DELAY_HEALTH_NOMINAL_MS);
            Serial.printf("[FDIR] OK | Bat: %.1f%% | Temp: %.1f C\n", currentBattery, currentTemp);
        }

        // 5. TELEMETRÍA: actualizar SOLO sensores en el snapshot.
        // El Mode Manager se encargará de actualizar 'mode', 'status' y 'error'.
        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            latestTelemetry.temperatureC   = currentTemp;
            latestTelemetry.batteryPercent = currentBattery;
            latestTelemetry.uptimeMs       = millis();
            xSemaphoreGive(telemetryMutex);
        } else {
            Serial.println("[FDIR] Advertencia: No se pudo obtener mutex. Telemetria de sensores no actualizada.");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}