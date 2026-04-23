#include "../include/csp_udp.h"
#include <WiFiUdp.h>
#include <Arduino.h>

static WiFiUDP udp;
static uint16_t current_local_port = 0;

void csp_udp_init(uint16_t local_udp_port) {
    if (current_local_port != local_udp_port) {
        udp.begin(local_udp_port);
        current_local_port = local_udp_port;
        Serial.printf("[CSP] UDP Listening on port %d\n", local_udp_port);
    }
}

bool csp_udp_send(uint8_t pri, uint8_t src, uint8_t dst, uint8_t sport, uint8_t dport, uint8_t *payload, uint16_t payload_len, IPAddress remote_ip, uint16_t remote_port) {
    csp_id_t id;
    id.ext = 0;
    id.pri = pri & 0x03;
    id.src = src & 0x1F;
    id.dst = dst & 0x1F;
    id.sport = sport & 0x3F;
    id.dport = dport & 0x3F;
    id.flags = 0;

    udp.beginPacket(remote_ip, remote_port);
    
    // Convert network byte order if necessary, ESP32 is little-endian.
    // For simplicity, we just send raw bytes of 32-bit uint in network order (Big Endian)
    uint32_t header_be = __builtin_bswap32(id.ext);
    
    udp.write((const uint8_t*)&header_be, sizeof(uint32_t));
    udp.write(payload, payload_len);
    
    return udp.endPacket() != 0;
}

bool csp_udp_recv(csp_packet_t *packet, uint32_t timeout_ms) {
    uint32_t start = millis();
    int packetSize = 0;
    
    // Wait for packet matching timeout
    do {
        packetSize = udp.parsePacket();
        if (packetSize > 0) break;
        if (timeout_ms > 0) {
#ifdef ESP32
            vTaskDelay(10 / portTICK_PERIOD_MS);
#else
            delay(10);
#endif
        }
    } while (millis() - start < timeout_ms);

    if (packetSize >= 4 && packetSize <= (4 + 256)) {
        uint32_t header_be;
        udp.read((char*)&header_be, sizeof(uint32_t));
        packet->id.ext = __builtin_bswap32(header_be);
        
        packet->length = packetSize - 4;
        if (packet->length > 0) {
            udp.read((char*)packet->data, packet->length);
        }
        return true;
    }
    
    return false;
}
