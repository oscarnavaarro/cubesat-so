#include "globals.h"
#include "../include/task_leds_pulsing.h"

void vTaskLedsPulsing(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz

    // Inicializar MPU6050
    if (!imu.begin()) {
        Serial.println("[LEDS] Error: No se pudo inicializar MPU6050");
        vTaskDelete(NULL);
        return;
    }
    
    Serial.println("[LEDS] MPU6050 inicializado. Coloca la placa de frente (norte) para sincronizar.");
    
    // El mensaje codificado se recupera de la MRAM (solo se aplica cuando está al norte)
    uint8_t mensaje[] = {1, 0, 0, 0, 0};
    int i = 0;
    
    // Umbral de tolerancia: ±15 grados (más flexible con filtro complementario)
    const float NORTH_ANGLE_MIN = 345.0f;  // -15°
    const float NORTH_ANGLE_MAX = 15.0f;   // +15°

    for (;;) {
        float yawAngle = imu.getYawAngle();
        
        // Verificar si está mirando al norte (ángulo 0)
        bool isNorth = (yawAngle >= NORTH_ANGLE_MIN) || (yawAngle <= NORTH_ANGLE_MAX);
        
        // Aplicar el patrón del mensaje solo cuando está mirando al norte
        if (isNorth) {
            digitalWrite(LED_PIN, mensaje[i]);
            i = (i + 1) % 5;
        } else {
            digitalWrite(LED_PIN, LOW);
            i = 0;  // Resetear índice cuando no esté al norte
        }
        
        // // Debug: mostrar ángulo cada 10 ciclos (~500ms)
        // static int debugCounter = 0;
        // if (++debugCounter >= 10) {
        //     Serial.printf("[LEDS] Yaw: %.1f° | North: %s\n", yawAngle, isNorth ? "YES" : "NO");
        //     debugCounter = 0;
        // }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    digitalWrite(LED_PIN, LOW);
    ledsPulsingHandle = NULL;
    vTaskDelete(NULL);
}