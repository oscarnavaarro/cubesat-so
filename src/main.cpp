#include "globals.h"
#include "battery.h"
#include "../include/task_health.h"
#include "../include/task_monitor.h"
#include "../include/task_mode_manager.h"
#include "../include/task_leds_pulsing.h"
#include "../include/task_post_deployment.h"
#include "../include/task_solver.h"

void setup() {
    Serial.begin(9600);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("\n[BOOT] Iniciando scheduler de tareas...");
    Serial.println("[BOOT] Control manual activo: w/s bateria +/-5, e/d temperatura +/-2");

    xTaskCreatePinnedToCore(vTaskMonitor, "Monitor", 4096, NULL, 3, &hMonitor, 1);
    // xTaskCreatePinnedToCore(vTaskHealth, "Checking", 3072, NULL, 3, &hChecking, 1);
    xTaskCreatePinnedToCore(vTaskModeManager, "ModeManager", 3072, NULL, 2, &hModeManager, 1);
    xTaskCreatePinnedToCore(vTaskLedsPulsing, "LedsPulsing", 3072, NULL, 2, &ledsPulsingHandle, 1);
    xTaskCreatePinnedToCore(vTaskPostDeployment, "PostDeploy", 3072, NULL, 1, &hPostDeploy, 1);
    Serial.println("[BOOT] Tareas creadas.");
}

void loop() {
    // El loop principal queda vacío, las tareas se ejecutan en paralelo
}