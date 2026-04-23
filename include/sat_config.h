#ifndef SAT_CONFIG_H
#define SAT_CONFIG_H

// ==========================================
// CONFIGURACIÓN DE RED Y RADIO (WIFI / CSP)
// ==========================================
static const char* const WIFI_SSID = "CubeSat_GS";
static const char* const WIFI_PASSWORD = "cubesat_password";

static const char* const GS_IP_ADDRESS = "192.168.1.1";
static const int GS_UDP_PORT = 12345;

static const char* const UPLINK_PSK = "cubesat_secret_key_123";

// ==========================================
// CONFIGURACIÓN DE HARDWARE Y PINES
// ==========================================
#define LED_PIN 2

// Macros Agnósticas de Hardware
#ifdef ESP32
    #include <Arduino.h>
    #define SYSTEM_RESET() ESP.restart()
#else
    // TODO: Implementar reset para Triskel / STM32
    #define SYSTEM_RESET() while(1) {} // Fallback seguro
#endif

// ==========================================
// CONFIGURACIÓN DEL RTOS (FreeRTOS)
// ==========================================
#define STACK_SIZE 512
#define PRIORITY_SUPERVISION 3

// Tiempos de las tareas (en milisegundos)
#define DELAY_DOWNLINK_MS 5000
#define DELAY_HEALTH_NOMINAL_MS 2000
#define DELAY_HEALTH_ALERT_MS 500
#define DELAY_MODE_MANAGER_MS 1000
#define TIMEOUT_UPLINK_RECV_MS 1000

// ==========================================
// UMBRALES DE FDIR (Fault Detection)
// ==========================================
#define TELEMETRY_BUFFER_SIZE 256

static constexpr float TEMP_SAFE_HIGH_THRESHOLD = 60.0f;
static constexpr float TEMP_COOLING_THRESHOLD = 45.0f;
static constexpr float TEMP_SAFE_LOW_THRESHOLD = -20.0f;

static constexpr float BATTERY_SAFE_THRESHOLD = 20.0f;
static constexpr float BATTERY_NOMINAL_THRESHOLD = 50.0f;

// ==========================================
// CONFIGURACIÓN DEL MONITOR DE SISTEMA
// ==========================================
#define DELAY_MONITOR_MS 5000
#define MONITOR_HEAP_CRITICAL_BYTES  (20U * 1024U)
#define MONITOR_STACK_CRITICAL_BYTES 256U

// Macro agnóstica para leer memoria libre del Heap
#ifdef ESP32
    #include <Arduino.h>
    #define HAL_GetFreeHeap() ESP.getFreeHeap()
#else
    // TODO: Implementar para Triskel
    #define HAL_GetFreeHeap() (0U)
#endif

#endif // SAT_CONFIG_H
