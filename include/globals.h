// Este archivo contiene las definiciones de variables globales y constantes utilizadas en todo el proyecto.

#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// Tamaño de stack recomendados (512 - 768 bytes)
#define STACK_SIZE 512
// Prioridades: El bloque de supervisión debe ser el más alto
#define PRIORITY_SUPERVISION 3

#define LED_PIN 2

extern QueueHandle_t modeQueue;
extern TaskHandle_t hMonitor;
extern TaskHandle_t hChecking;
extern TaskHandle_t hModeManager;
extern TaskHandle_t hPostDeploy;
extern TaskHandle_t ledsPulsingHandle;

typedef enum {
    MODE_NOMINAL,
    MODE_LOW_POWER,
    MODE_SAFE,
    MODE_COOLING,
} SatMode_t;

typedef enum {
    NO_ERROR = 0,
    OVERHEAT_ERROR,
    LOW_BATTERY_ERROR,
    COMPONENT_FAIL,
    PART_BROKEN,
    UNKNOWN_ERROR,
} SolverError_t;

extern SatMode_t currentMode;

#endif // GLOBALS_H