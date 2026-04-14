#include "globals.h"
#include "../include/task_post_deployment.h"

void vTaskPostDeployment(void *pvParameters) {
    Serial.printf("[DEPLOY] Esperando confirmación del despliegue físico...\n");

    // simulación de espera del timer por hardware (30mins en realidad)
    vTaskDelay(pdMS_TO_TICKS(5000));

    Serial.printf("[DEPLOY] Antenas desplegadas. Iniciando COMISSIONING...\n");
    currentMode = MODE_NOMINAL;

    hPostDeploy = NULL;
    vTaskDelete(NULL);
}