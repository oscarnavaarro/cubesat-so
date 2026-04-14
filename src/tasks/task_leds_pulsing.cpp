#include "globals.h"
#include "../include/task_leds_pulsing.h"

void vTaskLedsPulsing(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz por ejemplo

    // el mensaje codificado se recupera de la MRAM (8Mb)
    uint8_t mensaje[] = {1, 0, 0, 0, 0};
    int i = 0;

    for (;;) {
        // actuación sobre el driver de LEDs del payload
        digitalWrite(LED_PIN, mensaje[i]);
        i = (i + 1) % 5;
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    digitalWrite(LED_PIN, LOW);
    ledsPulsingHandle = NULL;
    vTaskDelete(NULL);
}