#include "globals.h"
#include "../include/task_monitor.h"
#include "../include/task_health.h"

constexpr TickType_t kMonitorPeriod = pdMS_TO_TICKS(5000);
constexpr uint32_t kHeapCriticalBytes = 20 * 1024;

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



void printTaskAudit(const char *name, TaskHandle_t handle, uint32_t stackDepthWords) {
    if (handle == NULL) {
        Serial.printf("Tarea: %s | Estado: NO CREADA | Consumo: N/A\n", name);
        return;
    }

    const UBaseType_t minFreeWords = uxTaskGetStackHighWaterMark(handle);
    const uint32_t stackTotalBytes = stackDepthWords * sizeof(StackType_t);
    const uint32_t minFreeBytes = static_cast<uint32_t>(minFreeWords) * sizeof(StackType_t);
    const uint32_t usedBytes = stackTotalBytes > minFreeBytes ? (stackTotalBytes - minFreeBytes) : 0;
    const float usedPercent = stackTotalBytes > 0
                                  ? (static_cast<float>(usedBytes) * 100.0f) / static_cast<float>(stackTotalBytes)
                                  : 0.0f;
    const eTaskState taskState = eTaskGetState(handle);

    Serial.printf(
        "Tarea: %s | Estado: %s | Consumo: %lu B (%.1f%%)\n",
        name,
        taskStateToString(taskState),
        static_cast<unsigned long>(usedBytes),
        static_cast<double>(usedPercent));

    if (minFreeBytes < 256) {
        Serial.printf("ALERTA: Stack critico en %s\n", name);
    }
}



void vTaskMonitor(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        Serial.println("\n--- MONITOR DE SISTEMA ---");

        const uint32_t freeHeap = ESP.getFreeHeap();

        Serial.printf("Memoria libre: %lu B\n", static_cast<unsigned long>(freeHeap));

        if (freeHeap < kHeapCriticalBytes) {
            Serial.println("ALERTA: Heap global bajo, riesgo de bloqueo/fragmentacion.");
        }

        // printTaskAudit("Monitor", hMonitor, 4096);
        // printTaskAudit("Checking", hChecking, 3072);
        // printTaskAudit("Solver", hSolver, 3072);
        // printTaskAudit("ModeManager", hModeManager, 3072);
        // printTaskAudit("LedsPulsing", ledsPulsingHandle, 3072);
        // printTaskAudit("PostDeploy", hPostDeploy, 3072);

        vTaskDelayUntil(&xLastWakeTime, kMonitorPeriod);
    }
}