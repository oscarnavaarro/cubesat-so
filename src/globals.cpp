#include "globals.h"

QueueHandle_t modeQueue = NULL;
TaskHandle_t hMonitor = NULL;
TaskHandle_t hChecking = NULL;
TaskHandle_t hModeManager = NULL;
TaskHandle_t hPostDeploy = NULL;
TaskHandle_t ledsPulsingHandle = NULL;

SatMode_t currentMode = MODE_NOMINAL;
