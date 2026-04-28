// MODE MANAGER - Máquina de Estados del Satélite
// Esta tarea es el ÚNICO punto de decisión sobre el modo operativo del satélite
// y su estado de salud global. Recibe eventos del FDIR (task_health) a través de la
// fdirQueue y comandos manuales del Uplink a través de commandedMode. 
// Aplica la jerarquía de seguridad y actualiza currentMode, healthStatus y la telemetría.

#include "globals.h"
#include "../include/task_mode_manager.h"
#include "../include/sat_config.h"

// Convierte un FdirEvent_t en el SatMode_t más restrictivo que exige.
static SatMode_t modeFromFdirEvent(FdirEvent_t event) {
    switch (event) {
        case FDIR_EVENT_OVERHEAT:
        case FDIR_EVENT_UNDERHEAT:
        case FDIR_EVENT_LOW_BATTERY:
            return MODE_SAFE;
        case FDIR_EVENT_HIGH_TEMP:
            return MODE_COOLING;
        case FDIR_EVENT_LOW_POWER_BATTERY:
            return MODE_LOW_POWER;
        case FDIR_EVENT_NOMINAL:
        default:
            return MODE_NOMINAL;
    }
}

// Devuelve el modo más restrictivo (mayor valor numérico = mayor prioridad de seguridad).
static SatMode_t mostRestrictiveMode(SatMode_t a, SatMode_t b) {
    return (static_cast<int>(a) >= static_cast<int>(b)) ? a : b;
}

void vTaskModeManager(void *pvParameters) {
    SatMode_t appliedMode = MODE_NOMINAL;
    bool payloadIsSuspended = false;
    SatMode_t fdirDemandedMode = MODE_NOMINAL; // Mantenemos estado entre ciclos

    for (;;) {
        // 1. CONSUMIR EVENTOS FDIR: vaciar la cola entera en cada ciclo.
        FdirEvent_t event;
        while (xQueueReceive(fdirQueue, &event, 0) == pdTRUE) {
            fdirDemandedMode = modeFromFdirEvent(event);
        }

        // 2. CONSIDERAR COMANDO MANUAL: el Uplink puede solicitar un cambio de modo.
        // La seguridad autónoma siempre tiene prioridad si es más restrictiva.
        const SatMode_t requestedMode = mostRestrictiveMode(fdirDemandedMode, commandedMode);

        // 3. DECISIÓN FINAL Y ESTADO DE SALUD
        if (requestedMode != appliedMode) {
            Serial.printf("[MODE_MGR] %s -> %s\n", modeToString(appliedMode), modeToString(requestedMode));
            currentMode = requestedMode;
            appliedMode = requestedMode;

            // Actualizar estado de salud global según el modo final
            if (requestedMode == MODE_SAFE) {
                healthStatus = HEALTH_NOK;
            } else {
                healthStatus = HEALTH_OK;
            }
        }

        // 4. ACTUALIZAR TELEMETRÍA: Guardamos los estados de decisión en el snapshot thread-safe
        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            latestTelemetry.mode = currentMode;
            latestTelemetry.status = healthStatus;
            // healthError se actualiza en el Solver, así que también lo persistimos aquí
            latestTelemetry.error = healthError; 
            xSemaphoreGive(telemetryMutex);
        } else {
            Serial.println("[MODE_MGR] Advertencia: No se pudo obtener mutex. Telemetria de estados no actualizada.");
        }

        // 5. ACTUADORES: gestionar la suspensión de la tarea de payload según el modo
        const bool shouldSuspendPayload = (currentMode != MODE_NOMINAL);
        if (ledsPulsingHandle && shouldSuspendPayload != payloadIsSuspended) {
            if (shouldSuspendPayload) {
                vTaskSuspend(ledsPulsingHandle);
                Serial.printf("[MODE_MGR] Payload suspendido (modo: %s)\n", modeToString(currentMode));
            } else {
                vTaskResume(ledsPulsingHandle);
                Serial.println("[MODE_MGR] Payload reanudado (modo NOMINAL)");
            }
            payloadIsSuspended = shouldSuspendPayload;
        }

        vTaskDelay(pdMS_TO_TICKS(DELAY_MODE_MANAGER_MS));
    }
}