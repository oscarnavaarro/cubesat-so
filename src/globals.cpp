#include "globals.h"

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

QueueHandle_t modeQueue = NULL;
TaskHandle_t hMonitor = NULL;
TaskHandle_t hChecking = NULL;
TaskHandle_t hSolver = NULL;
TaskHandle_t hModeManager = NULL;
TaskHandle_t hPostDeploy = NULL;
TaskHandle_t ledsPulsingHandle = NULL;

SatMode_t currentMode = MODE_NOMINAL;
volatile HealthStatus_t healthStatus = HEALTH_OK;
volatile SolverError_t healthError = NO_ERROR;
volatile SatMode_t healthProposedMode = MODE_NOMINAL;

MPU6050 imu;
