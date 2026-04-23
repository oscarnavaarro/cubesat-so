#ifndef CSP_UDP_H
#define CSP_UDP_H

#include <stdint.h>
#include <stdbool.h>

// Simulamos la cabecera CSP estándar de 32-bits (versión 1)
typedef union {
    uint32_t ext;
    struct {
        uint32_t pri : 2;
        uint32_t src : 5;
        uint32_t dst : 5;
        uint32_t dport : 6;
        uint32_t sport : 6;
        uint32_t flags : 8;
    };
} csp_id_t;

typedef struct {
    csp_id_t id;
    uint16_t length;
    uint8_t data[256];
} csp_packet_t;

// Direcciones predeterminadas para nuestro entorno
#define CSP_NODE_OBC 1
#define CSP_NODE_GS 2

#define CSP_PORT_TELEMETRY 10
#define CSP_PORT_COMMANDS 20

// Inicializa el socket UDP apuntando al puerto RX local
void csp_udp_init(uint16_t local_udp_port);

#include <IPAddress.h>

// Emite un paquete usando estructura CSP sobre WiFi UDP
bool csp_udp_send(uint8_t pri, uint8_t src, uint8_t dst, uint8_t sport, uint8_t dport, uint8_t *payload, uint16_t payload_len, IPAddress remote_ip, uint16_t remote_port);

// Recibe por UDP reconstruyendo la estructura CSP
bool csp_udp_recv(csp_packet_t *packet, uint32_t timeout_ms);

#endif
