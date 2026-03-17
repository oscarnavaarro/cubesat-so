#ifndef TASK_HEALTH_H
#define TASK_HEALTH_H

// Definición de buffers para telemetría, simulando almacenamiento en NAND Flash/SDRAM
#define TELEMETRY_BUFFER_SIZE 256
uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE];
bool systemOK = true; // OK/NOK

void vTaskHealth(void *pvParameters);

#endif // TASK_HEALTH_H