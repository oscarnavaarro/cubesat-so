#include "../include/task_uplink.h"
#include "../include/globals.h"
#include "../include/csp_udp.h"
#include "../include/sat_config.h"
#include "sat.pb.h"
#include <pb_decode.h>
#include <mbedtls/md.h>
#include <Arduino.h>

TaskHandle_t hUplink = NULL;

static bool verify_hmac(const uint8_t* payload, size_t payload_len, const uint8_t* received_mac) {
    uint8_t mac[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)UPLINK_PSK, strlen(UPLINK_PSK));
    mbedtls_md_hmac_update(&ctx, payload, payload_len);
    mbedtls_md_hmac_finish(&ctx, mac);
    mbedtls_md_free(&ctx);

    return memcmp(mac, received_mac, 32) == 0;
}

void vTaskUplink(void *pvParameters) {
    csp_packet_t packet;
    
    for (;;) {
        // Bloqueante por defecto corto-tiempo en la recolección
        if (csp_udp_recv(&packet, 1000)) { 
            if (packet.id.dport == CSP_PORT_COMMANDS) {
                SecureCommand secure_msg = SecureCommand_init_zero;
                pb_istream_t stream = pb_istream_from_buffer(packet.data, packet.length);
                
                if (!pb_decode(&stream, SecureCommand_fields, &secure_msg)) {
                    Serial.printf("[UPLINK] Decode error: %s\n", PB_GET_ERROR(&stream));
                    continue;
                }
                
                // secure_msg decodificado. Extraer HMAC validando contra MAC recibido
                if (!verify_hmac(secure_msg.payload.bytes, secure_msg.payload.size, secure_msg.mac.bytes)) {
                    Serial.println("[UPLINK] Fallo HMAC, se deniega acceso al comando (Posible atacante).");
                    continue;
                }
                
                // Decodificar el Payload (Command object interior)
                Command cmd = Command_init_zero;
                pb_istream_t cmd_stream = pb_istream_from_buffer(secure_msg.payload.bytes, secure_msg.payload.size);
                
                if (pb_decode(&cmd_stream, Command_fields, &cmd)) {
                    Serial.printf("[UPLINK] ✅ Comando Autenticado. Accion: %d, TargetMode: %d, Nonce: %d\n", 
                                  cmd.action, cmd.target_mode, cmd.nonce);
                    
                    if (cmd.action == Command_Action_RESTART) {
                        Serial.println("[UPLINK] Ejecutando orden RESET desde Tierra...");
                        vTaskDelay(200 / portTICK_PERIOD_MS);
                        SYSTEM_RESET();
                    } else if (cmd.action == Command_Action_CHANGE_MODE) {
                        SatMode_t t_mode = (SatMode_t)cmd.target_mode;
                        commandedMode = t_mode;
                        Serial.printf("[UPLINK] Solicitando cambio manual de modo a: %s\n", modeToString(t_mode));
                    }
                } else {
                    Serial.println("[UPLINK] Fallo decodificando payload interno validado.");
                }
            }
        }
    }
}
