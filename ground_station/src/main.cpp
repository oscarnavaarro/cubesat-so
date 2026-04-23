#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "csp_udp.h"
#include "sat.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <bearssl/bearssl_hmac.h>
#include <bearssl/bearssl_hash.h>

const char* ssid = "CubeSat_GS";
const char* password = "cubesat_password";
static const char* PSK = "cubesat_secret_key_123";

uint32_t current_nonce = 1;

void calculate_hmac(const uint8_t* payload, size_t payload_len, uint8_t* mac_out) {
    br_hmac_key_context kc;
    br_hmac_context hc;
    br_hmac_key_init(&kc, &br_sha256_vtable, PSK, strlen(PSK));
    br_hmac_init(&hc, &kc, 0);
    br_hmac_update(&hc, payload, payload_len);
    br_hmac_out(&hc, mac_out);
}

void send_command(Command_Action action, uint32_t target_mode) {
    Serial.println("[GS-DEBUG] Iniciando send_command...");
    // 1. Encode Inner Command
    Command cmd = Command_init_zero;
    cmd.action = action;
    cmd.target_mode = target_mode;
    cmd.nonce = current_nonce++;

    // Force 32-bit alignment for BearSSL compatibility
    uint32_t cmd_buffer_align[128 / 4];
    uint8_t* cmd_buffer = (uint8_t*)cmd_buffer_align;

    pb_ostream_t cmd_stream = pb_ostream_from_buffer(cmd_buffer, 128);
    if (!pb_encode(&cmd_stream, Command_fields, &cmd)) {
        Serial.println("[GS] Error al codificar el Comando interno.");
        return;
    }

    Serial.println("[GS-DEBUG] Comando interno codificado. Calculando HMAC...");
    // 2. Calculate HMAC
    uint32_t mac_align[32 / 4];
    uint8_t* mac = (uint8_t*)mac_align;
    calculate_hmac(cmd_buffer, cmd_stream.bytes_written, mac);

    Serial.println("[GS-DEBUG] HMAC calculado. Creando SecureCommand...");
    // 3. Create and Encode SecureCommand
    SecureCommand secure_msg = SecureCommand_init_zero;
    memcpy(secure_msg.payload.bytes, cmd_buffer, cmd_stream.bytes_written);
    secure_msg.payload.size = cmd_stream.bytes_written;
    memcpy(secure_msg.mac.bytes, mac, 32);
    secure_msg.mac.size = 32;



    uint32_t secure_buffer_align[256 / 4];
    uint8_t* secure_buffer = (uint8_t*)secure_buffer_align;
    pb_ostream_t secure_stream = pb_ostream_from_buffer(secure_buffer, 256);
    if (!pb_encode(&secure_stream, SecureCommand_fields, &secure_msg)) {
        Serial.printf("[GS] Error al codificar el SecureCommand: %s\n", PB_GET_ERROR(&secure_stream));
        return;
    }

    Serial.println("[GS-DEBUG] SecureCommand codificado. Enviando por UDP...");
    // 4. Send via CSP UDP (Broadcast to the AP subnet on port 20)
    IPAddress broadcast_ip(192, 168, 1, 255);
    bool sent = csp_udp_send(1, CSP_NODE_GS, CSP_NODE_OBC, CSP_PORT_COMMANDS, CSP_PORT_COMMANDS, 
                             secure_buffer, secure_stream.bytes_written, 
                             broadcast_ip, 20);
    
    if (sent) {
        Serial.printf("[GS] Comando enviado con éxito. Acción: %d, Modo destino: %d\n", (int)action, (int)target_mode);
        Serial.print("[GS-DEBUG] HEX: ");
        for(size_t i=0; i<secure_stream.bytes_written; i++) {
            Serial.printf("%02X ", secure_buffer[i]);
        }
        Serial.println();
    } else {
        Serial.println("[GS] Fallo al enviar comando por UDP.");
    }
}

void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("\n\n[GS] Iniciando Estación Terrestre...");

    // Configurar Punto de Acceso (AP)
    IPAddress local_IP(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid, password);
    
    Serial.print("[GS] AP Iniciado. IP: ");
    Serial.println(WiFi.softAPIP());

    // Iniciar escucha UDP para Telemetría (El satélite envía a 12345 en task_downlink.cpp)
    csp_udp_init(12345);

    Serial.println("\n[GS] Listo. Esperando telemetría del satélite...");
    Serial.println("=================================================");
    Serial.println(" CONTROLES DE LA ESTACIÓN TERRESTRE: ");
    Serial.println(" [1] Enviar Comando RESTART al Satélite");
    Serial.println(" [2] Enviar Comando CHANGE_MODE (Modo SAFE)");
    Serial.println(" [3] Enviar Comando CHANGE_MODE (Modo NOMINAL)");
    Serial.println("=================================================\n");
}

void loop() {
    // Procesar entrada por puerto serie para enviar comandos
    if (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '1') {
            send_command(Command_Action_RESTART, 0);
        } else if (c == '2') {
            send_command(Command_Action_CHANGE_MODE, 3); // Asumiendo 3 es MODE_SAFE
        } else if (c == '3') {
            send_command(Command_Action_CHANGE_MODE, 0); // Asumiendo 0 es MODE_NOMINAL
        }
    }

    // Recibir paquetes CSP (Telemetría)
    csp_packet_t packet;
    if (csp_udp_recv(&packet, 10)) { // Timeout pequeño para no bloquear
        // Verificamos si es un paquete de telemetría (puerto 10 según csp_udp.h, aunque en main.cpp se manda al 12345 y con dport CSP_PORT_TELEMETRY)
        if (packet.id.dport == CSP_PORT_TELEMETRY) {
            Telemetry t = Telemetry_init_zero;
            pb_istream_t stream = pb_istream_from_buffer(packet.data, packet.length);
            
            if (pb_decode(&stream, Telemetry_fields, &t)) {
                Serial.printf("[TELEMETRÍA] Uptime: %d ms | Batería: %d%% | Temp: %.1f C | Modo: %d | Salud: %d\n",
                              (int)t.uptimeMs, (int)t.batteryPercent, (float)t.temperatureC, (int)t.mode, (int)t.status);
            } else {
                Serial.printf("[GS] Error decodificando telemetría: %s\n", PB_GET_ERROR(&stream));
            }
        }
    }
}
