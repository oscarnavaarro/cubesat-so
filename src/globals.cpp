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
QueueHandle_t fdirQueue = NULL;
TaskHandle_t hMonitor = NULL;
TaskHandle_t hChecking = NULL;
TaskHandle_t hSolver = NULL;
TaskHandle_t hModeManager = NULL;
TaskHandle_t hPostDeploy = NULL;
TaskHandle_t ledsPulsingHandle = NULL;

SatMode_t currentMode = MODE_NOMINAL;
volatile HealthStatus_t healthStatus = HEALTH_OK;
volatile SolverError_t healthError = NO_ERROR;
volatile SatMode_t commandedMode = MODE_NOMINAL;

volatile float lastTemperature = 0.0f;
volatile float lastBattery = 0.0f;
volatile uint32_t systemUptime = 0;

// Mutex y snapshot de telemetría thread-safe.
// El mutex se crea en main.cpp antes de lanzar las tareas.
SemaphoreHandle_t telemetryMutex = NULL;
TelemetrySnapshot_t latestTelemetry = {0.0f, 0.0f, 0, MODE_NOMINAL, HEALTH_OK, NO_ERROR};

MPU6050 imu;
