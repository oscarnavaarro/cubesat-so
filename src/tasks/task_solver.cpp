#include "globals.h"
#include "../include/task_solver.h"

namespace {
SatMode_t solveModeFromHealth(HealthStatus_t status, SolverError_t error, SatMode_t proposedMode) {
    if (status == HEALTH_NOK && proposedMode != MODE_SAFE) {
        switch (error) {
            case LOW_BATTERY_ERROR:
            case OVERHEAT_ERROR:
            case COMPONENT_FAIL:
            case PART_BROKEN:
            case UNKNOWN_ERROR:
            default:
                return MODE_SAFE;
        }
    }

    return proposedMode;
}
} // namespace

void vTaskSolver(void *pvParameters) {
    SatMode_t lastPublishedMode = currentMode;

    for (;;) {
        const SatMode_t solvedMode = solveModeFromHealth(healthStatus, healthError, healthProposedMode);
        currentMode = solvedMode;

        if (solvedMode != lastPublishedMode) {
            Serial.printf("[SOLVER] Modo decidido: %d (health=%d, error=%d)\n",
                          static_cast<int>(solvedMode),
                          static_cast<int>(healthStatus),
                          static_cast<int>(healthError));
            lastPublishedMode = solvedMode;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}