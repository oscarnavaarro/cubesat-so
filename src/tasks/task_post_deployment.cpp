#include "globals.h"
#include "../include/task_post_deployment.h"

void vTaskPostDeployment(void *pvParameters) {
    Serial.printf("[DEPLOY] Esperando confirmación del despliegue físico...\n");

    // Simulación de espera del timer por hardware (ej. 30mins en realidad)
    vTaskDelay(pdMS_TO_TICKS(5000)); // Simulación de espera

    Serial.printf("[DEPLOY] Antenas desplegadas. Iniciando COMISSIONING...\n");
    currentMode = MODE_NOMINAL;

    // Limpia el handle global antes de autodestruirse para evitar referencias obsoletas.
    hPostDeploy = NULL;

    // Cuando termine la misión la tarea o se elimina o se queda inactiva
    vTaskDelete(NULL);
}