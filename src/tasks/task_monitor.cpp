#include "globals.h"
#include "../include/task_monitor.h"
#include "../include/task_health.h"

#if __has_include(<time.h>)
#include <time.h>
#define HAS_PORTABLE_TIME_API 1
#else
#define HAS_PORTABLE_TIME_API 0
#endif

constexpr TickType_t kMonitorPeriod = pdMS_TO_TICKS(5000);
constexpr uint32_t kHeapCriticalBytes = 20 * 1024;

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

const char *taskStateToString(eTaskState state) {
    switch (state) {
        case eRunning:
            return "RUNNING";
        case eReady:
            return "READY";
        case eBlocked:
            return "BLOCKED";
        case eSuspended:
            return "SUSPENDED";
        case eDeleted:
            return "DELETED";
        default:
            return "INVALID";
    }
}

bool formatCurrentSatelliteTime(char *buffer, size_t bufferLen) {
#if HAS_PORTABLE_TIME_API
    const time_t nowSecs = time(nullptr);

    if (nowSecs <= 0) {
        return false;
    }

    struct tm currentTm;
    if (localtime_r(&nowSecs, &currentTm) == nullptr) {
        return false;
    }

    const size_t charsWritten = strftime(buffer, bufferLen, "%Y-%m-%d %H:%M:%S", &currentTm);
    return charsWritten > 0;
#else
    (void)buffer;
    (void)bufferLen;
    return false;
#endif
}

void printTaskAudit(const char *name, TaskHandle_t handle, uint32_t stackDepthWords) {
    if (handle == NULL) {
        Serial.printf("Tarea: %s | Estado: NO CREADA\n", name);
        return;
    }

    const UBaseType_t minFreeWords = uxTaskGetStackHighWaterMark(handle);
    const uint32_t stackTotalBytes = stackDepthWords * sizeof(StackType_t);
    const uint32_t minFreeBytes = static_cast<uint32_t>(minFreeWords) * sizeof(StackType_t);
    const uint32_t usedBytes = stackTotalBytes > minFreeBytes ? (stackTotalBytes - minFreeBytes) : 0;
    const eTaskState taskState = eTaskGetState(handle);

    Serial.printf(
        "Tarea: %s | Estado: %s | Stack total: %lu B | Min libre: %lu B | Pico usado: %lu B\n",
        name,
        taskStateToString(taskState),
        static_cast<unsigned long>(stackTotalBytes),
        static_cast<unsigned long>(minFreeBytes),
        static_cast<unsigned long>(usedBytes));

    if (minFreeBytes < 256) {
        Serial.printf("ALERTA: Stack critico en %s\n", name);
    }
}



void vTaskMonitor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        Serial.println("\n--- MONITOR DE SISTEMA ---");

        char currentTime[32] = {0};
        if (formatCurrentSatelliteTime(currentTime, sizeof(currentTime))) {
            Serial.printf("Hora satelite: %s\n", currentTime);
        } else {
            Serial.println("Hora satelite: no disponible (sincronizacion o API de tiempo pendiente)");
            // Si en Triskel no existe <time.h>, reemplazar con la API RTC especifica de la placa.
        }

        const uint32_t freeHeap = ESP.getFreeHeap();
        const uint32_t minFreeHeap = ESP.getMinFreeHeap();
        const uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
        const float currentTemp = readInternalTemp();
        const float currentBattery = readBatteryLevel();

        Serial.printf("Modo actual: %s\n", modeToString(currentMode));
        Serial.printf("Bateria: %.1f%% | Temperatura: %.1f C\n", currentBattery, currentTemp);
        Serial.printf("Heap libre total: %lu B | Min historico: %lu B | Bloque maximo: %lu B\n",
                      static_cast<unsigned long>(freeHeap),
                      static_cast<unsigned long>(minFreeHeap),
                      static_cast<unsigned long>(maxAllocHeap));

        if (freeHeap < kHeapCriticalBytes) {
            Serial.println("ALERTA: Heap global bajo, riesgo de bloqueo/fragmentacion.");
        }

        printTaskAudit("Monitor", hMonitor, 4096);
        printTaskAudit("Checking", hChecking, 3072);
        printTaskAudit("ModeManager", hModeManager, 3072);
        printTaskAudit("LedsPulsing", ledsPulsingHandle, 3072);
        printTaskAudit("PostDeploy", hPostDeploy, 3072);

        vTaskDelayUntil(&xLastWakeTime, kMonitorPeriod);
    }
}