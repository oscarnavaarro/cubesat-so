#include "../include/task_downlink.h"
#include "../include/globals.h"
#include "../include/csp_udp.h"
#include "sat.pb.h"
#include <pb_encode.h>
#include <Arduino.h>

TaskHandle_t hDownlink = NULL;

// IP de la estación terrestre, asumiendo AP modo WiFI
const char* gs_ip = "192.168.1.1"; 

void vTaskDownlink(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Prioridad baja: envía por la ventana/loop cada 5 seg
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); 
    
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Leer datos thread-safe si es posible, aquí usamos los globales
        Telemetry telemetry_msg = Telemetry_init_zero;
        telemetry_msg.uptimeMs = systemUptime;
        telemetry_msg.temperatureC = lastTemperature;
        telemetry_msg.batteryPercent = lastBattery;
        // Casteos seguros a uint32 para nanopb
        telemetry_msg.mode = (uint32_t)currentMode;
        telemetry_msg.status = (uint32_t)healthStatus;
        telemetry_msg.error = (uint32_t)healthError;

        uint8_t buffer[128];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        
        bool status = pb_encode(&stream, Telemetry_fields, &telemetry_msg);
        
        if (!status) {
            Serial.printf("[DOWNLINK] Error codificando Telemetry: %s\n", PB_GET_ERROR(&stream));
            continue;
        }

        // Enviar vía CSP sobre UDP
        bool sent = csp_udp_send(1, CSP_NODE_OBC, CSP_NODE_GS, CSP_PORT_TELEMETRY, 
                                CSP_PORT_TELEMETRY, buffer, stream.bytes_written, 
                                gs_ip, 12345);
        if (sent) {
            Serial.printf("[DOWNLINK] Telemetría enviada a GS: %d bytes.\n", stream.bytes_written);
        } else {
            Serial.println("[DOWNLINK] Advertencia: Error en capa UDP. ¿Hay red conectada?");
        }
    }
}
