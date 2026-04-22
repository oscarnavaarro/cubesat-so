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

const char* ssid = "CubeSat_GS";
const char* password = "cubesat_password";

void setup() {
    Serial.begin(9600);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Serial.println("\n[BOOT] Iniciando sistema de radio...");
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("\n[BOOT] Conectado al WiFi de la Estación Terrestre");

    // csp_udp_init(CSP_PORT_COMMANDS);

    Serial.println("[BOOT] Iniciando scheduler de tareas...");
    Serial.println("[BOOT] Control manual activo: w/s bateria +/-5, e/d temperatura +/-2");

    xTaskCreatePinnedToCore(vTaskMonitor, "Monitor", 3072, NULL, 3, &hMonitor, 1);
    xTaskCreatePinnedToCore(vTaskHealth, "Checking", 3072, NULL, 3, &hChecking, 1);
   // xTaskCreatePinnedToCore(vTaskUplink, "Uplink", 3072, NULL, 3, &hUplink, 0); // Core 0 = Radio Rx (Alto rend)
    xTaskCreatePinnedToCore(vTaskSolver, "Solver", 3072, NULL, 2, &hSolver, 1);
    xTaskCreatePinnedToCore(vTaskModeManager, "ModeManager", 2048, NULL, 2, &hModeManager, 1);
    xTaskCreatePinnedToCore(vTaskLedsPulsing, "LedsPulsing", 2048, NULL, 2, &ledsPulsingHandle, 1);
    // xTaskCreatePinnedToCore(vTaskDownlink, "Downlink", 3072, NULL, 1, &hDownlink, 0); // Core 0 = Radio Tx (Baja prio)
    xTaskCreatePinnedToCore(vTaskPostDeployment, "PostDeploy", 2048, NULL, 1, &hPostDeploy, 1);
    
    Serial.println("[BOOT] Tareas creadas.");
}

void loop() {
    // El loop principal queda vacío, las tareas se ejecutan en paralelo
}