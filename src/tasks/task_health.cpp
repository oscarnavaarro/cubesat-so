#include "globals.h"
#include "../include/task_health.h"
#include "../include/task_solver.h"
#include "../include/battery.h"
#include <SPIFFS.h>
#include <string.h>

uint8_t telemetryBuffer[TELEMETRY_BUFFER_SIZE] = {0};
bool systemOK = true;

namespace {
struct TelemetryFrame {
    uint32_t uptimeMs;
    float temperatureC;
    float batteryPercent;
    uint8_t status;
    uint8_t crc;
};

constexpr uint8_t kXorKey = 0x5A;

uint8_t computeXorCrc(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
    }
    return crc;
}
} // namespace

float readInternalTemp(void) {
    return static_cast<float>(readManualTemperature());
}

float readBatteryLevel(void) {
    return static_cast<float>(simulateBatteryLevel());
}

void packAndEncryptTelemetry(float currentTemp, float currentBattery, bool isSystemOK) {
    TelemetryFrame frame{};
    frame.uptimeMs = millis();
    frame.temperatureC = currentTemp;
    frame.batteryPercent = currentBattery;
    frame.status = isSystemOK ? 1 : 0;

    uint8_t raw[sizeof(TelemetryFrame)] = {0};
    memcpy(raw, &frame, sizeof(TelemetryFrame));
    frame.crc = computeXorCrc(raw, sizeof(TelemetryFrame) - 1);
    memcpy(raw, &frame, sizeof(TelemetryFrame));

    for (size_t i = 0; i < sizeof(TelemetryFrame); ++i) {
        raw[i] ^= kXorKey;
    }

    const size_t copyLen = sizeof(TelemetryFrame) <= TELEMETRY_BUFFER_SIZE ? sizeof(TelemetryFrame)
                                                                            : TELEMETRY_BUFFER_SIZE;
    memcpy(telemetryBuffer, raw, copyLen);

    static bool storageReady = false;
    if (!storageReady) {
        storageReady = SPIFFS.begin(true);
        if (!storageReady) {
            Serial.println("[HEALTH] No se pudo inicializar SPIFFS para telemetria de prueba.");
        }
    }

    if (storageReady) {
        File telemetryFile = SPIFFS.open("/telemetry_mock.bin", FILE_APPEND);
        if (telemetryFile) {
            telemetryFile.write(raw, sizeof(TelemetryFrame));
            telemetryFile.close();
        } else {
            Serial.println("[HEALTH] No se pudo abrir /telemetry_mock.bin");
        }
    }
}

void vTaskHealth(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // Frecuencia dinámica: nominal cada 2s, anomalía 500ms
    TickType_t xFrequency = pdMS_TO_TICKS(2000);

    for (;;) {
        // 1. RECOPILACIÓN (telemetría externa e interna)
        // se lee toda la información disponible (temperatura, batería, paneles, etc)
        float currentTemp = readInternalTemp();
        float currentBattery = readBatteryLevel();

        // 2. CHECKING (generación de OK/NOK)
        // el gestor de salud trabaja con toda la información disponible
        if (currentTemp > MAX_TEMP_THRESHOLD || currentBattery < MIN_BATTERY_THRESHOLD) {
            systemOK = false;
            Serial.printf("[HEALTH] Estado: NOK detectado!\n");

            // 3. LLAMADA A SOLVER (acción inmediata)
            vSolver(currentTemp > MAX_TEMP_THRESHOLD ? OVERHEAT_ERROR : LOW_BATTERY_ERROR);
            // en este ejemplo, el solver simplemente imprime un mensaje, pero podría ser una acción más compleja 
            // como activar un sistema de enfriamiento o reducir la carga del sistema.

            xFrequency = pdMS_TO_TICKS(500); // aumentar frecuencia en caso de anomalía
        } else {
            systemOK = true;
            xFrequency = pdMS_TO_TICKS(2000); // frecuencia nominal
        }

        // 4. STATUS REPORT (empaquetado y cifrado)
        // se genera el reporte completo y se deja en el buffer para Downlink
        packAndEncryptTelemetry(currentTemp, currentBattery, systemOK);
        Serial.printf("[HEALTH] Telemetría generada y cifrada en buffer.\n");

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}