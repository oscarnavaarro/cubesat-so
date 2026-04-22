// Este archivo contiene las definiciones de variables globales y constantes utilizadas en todo el proyecto.

#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "mpu6050.h"

// Tamaño de stack recomendados (512 - 768 bytes)
#define STACK_SIZE 512
// Prioridades: El bloque de supervisión debe ser el más alto
#define PRIORITY_SUPERVISION 3

#define LED_PIN 2

extern QueueHandle_t modeQueue;
extern TaskHandle_t hMonitor;
extern TaskHandle_t hChecking;
extern TaskHandle_t hSolver;
extern TaskHandle_t hModeManager;
extern TaskHandle_t hPostDeploy;
extern TaskHandle_t ledsPulsingHandle;

typedef enum {
    MODE_NOMINAL = 0,
    MODE_LOW_POWER = 1,
    MODE_COOLING = 2,
    MODE_SAFE = 3,
} SatMode_t;

typedef enum {
    NO_ERROR = 0,
    OVERHEAT_ERROR,
    LOW_BATTERY_ERROR,
    COMPONENT_FAIL,
    PART_BROKEN,
    UNKNOWN_ERROR,
} SolverError_t;

typedef enum {
    HEALTH_OK = 0,
    HEALTH_NOK = 1,
} HealthStatus_t;

extern SatMode_t currentMode;
extern volatile HealthStatus_t healthStatus;
extern volatile SolverError_t healthError;
extern volatile SatMode_t healthProposedMode;

extern volatile float lastTemperature;
extern volatile float lastBattery;
extern volatile uint32_t systemUptime;

extern MPU6050 imu;

const char *modeToString(SatMode_t mode);

#endif // GLOBALS_H