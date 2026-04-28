#include "../include/task_downlink.h"
#include "../include/globals.h"
#include "../include/csp_udp.h"
#include "../include/sat_config.h"
#include "sat.pb.h"
#include <pb_encode.h>
#include <Arduino.h>

TaskHandle_t hDownlink = NULL;

void vTaskDownlink(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Prioridad baja: envía por la ventana/loop cada N seg. Dinámico según el modo.
    TickType_t xFrequency = pdMS_TO_TICKS(DELAY_DOWNLINK_MS);
    
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Copiar el snapshot de telemetría de forma thread-safe antes de serializar.
        // Se usa una copia local para minimizar el tiempo que el mutex está bloqueado.
        TelemetrySnapshot_t snap;
        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            snap = latestTelemetry;
            xSemaphoreGive(telemetryMutex);
        } else {
            Serial.println("[DOWNLINK] Advertencia: No se pudo obtener mutex. Ciclo omitido.");
            continue;
        }

        Telemetry telemetry_msg = Telemetry_init_zero;
        telemetry_msg.uptimeMs       = snap.uptimeMs;
        telemetry_msg.temperatureC   = snap.temperatureC;
        telemetry_msg.batteryPercent = snap.batteryPercent;
        telemetry_msg.mode           = (uint32_t)snap.mode;
        telemetry_msg.status         = (uint32_t)snap.status;
        telemetry_msg.error          = (uint32_t)snap.error;

        // Ajustar frecuencia del próximo ciclo según el modo actual
        if (snap.mode == MODE_SAFE) {
            xFrequency = pdMS_TO_TICKS(DELAY_DOWNLINK_ALERT_MS);
        } else {
            xFrequency = pdMS_TO_TICKS(DELAY_DOWNLINK_MS);
        }

        uint8_t buffer[128];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        const bool encoded = pb_encode(&stream, Telemetry_fields, &telemetry_msg);

        if (!encoded) {
            Serial.printf("[DOWNLINK] Error codificando Telemetry: %s\n", PB_GET_ERROR(&stream));
            continue;
        }

        // Enviar vía CSP sobre UDP
        const bool sent = csp_udp_send(1, CSP_NODE_OBC, CSP_NODE_GS, CSP_PORT_TELEMETRY,
                                CSP_PORT_TELEMETRY, buffer, stream.bytes_written,
                                GS_IP_ADDRESS, GS_UDP_PORT);
        if (sent) {
            Serial.printf("[DOWNLINK] Telemetría enviada a GS: %d bytes.\n", stream.bytes_written);
        } else {
            Serial.println("[DOWNLINK] Advertencia: Error en capa UDP. ¿Hay red conectada?");
        }
    }
}
