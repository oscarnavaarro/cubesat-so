#include "globals.h"
#include "battery.h"
#include <WiFi.h>
#include "../include/task_health.h"
#include "../include/task_monitor.h"
#include "../include/task_mode_manager.h"
#include "../include/task_leds_pulsing.h"
#include "../include/task_post_deployment.h"
#include "../include/task_solver.h"
#include "../include/task_downlink.h"
#include "../include/task_uplink.h"
#include "../include/csp_udp.h"
#include "sat_config.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("\n[BOOT] Iniciando sistema de radio...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[BOOT] Conectado al WiFi de la Estación Terrestre");

    WiFi.setSleep(false); 

    csp_udp_init(CSP_PORT_COMMANDS);

    Serial.println("[BOOT] Iniciando scheduler de tareas...");
    Serial.println("[BOOT] Control manual activo: w/s bateria +/-5, e/d temperatura +/-2");

    // Crear primitivas de sincronización antes de lanzar las tareas
    telemetryMutex = xSemaphoreCreateMutex();
    if (telemetryMutex == NULL) {
        Serial.println("[BOOT] FATAL: No se pudo crear el mutex de telemetria.");
        SYSTEM_RESET();
    }

    // Cola FDIR: capacidad para 8 eventos FdirEvent_t.
    fdirQueue = xQueueCreate(8, sizeof(FdirEvent_t));
    if (fdirQueue == NULL) {
        Serial.println("[BOOT] FATAL: No se pudo crear la cola FDIR.");
        SYSTEM_RESET();
    }

    xTaskCreate(vTaskMonitor, "Monitor", 3072, NULL, 3, &hMonitor);
    xTaskCreate(vTaskHealth, "Checking", 3072, NULL, 3, &hChecking);
    xTaskCreate(vTaskUplink, "Uplink", 3072, NULL, 3, &hUplink);
    xTaskCreate(vTaskSolver, "Solver", 3072, NULL, 2, &hSolver);
    xTaskCreate(vTaskModeManager, "ModeManager", 2048, NULL, 2, &hModeManager);
    xTaskCreate(vTaskLedsPulsing, "LedsPulsing", 2048, NULL, 2, &ledsPulsingHandle);
    xTaskCreate(vTaskDownlink, "Downlink", 3072, NULL, 1, &hDownlink);
    xTaskCreate(vTaskPostDeployment, "PostDeploy", 2048, NULL, 1, &hPostDeploy);
    
    Serial.println("[BOOT] Tareas creadas.");
}

void loop() {
    // El loop principal queda vacío, las tareas se ejecutan en paralelo
}