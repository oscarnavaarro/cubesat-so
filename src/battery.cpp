#include "battery.h"
#include <Arduino.h>

namespace {
int gBatteryLevel = 60;
int gTemperatureC = 30;

constexpr int kBatteryMin = 0;
constexpr int kBatteryMax = 100;
constexpr int kTemperatureMin = -40;
constexpr int kTemperatureMax = 120;

int clampInt(int value, int minValue, int maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}
} // namespace

int simulateBatteryLevel(void) {
    return gBatteryLevel;
}

int readManualTemperature(void) {
    return gTemperatureC;
}

void adjustBatteryByStep(int delta) {
    gBatteryLevel = clampInt(gBatteryLevel + delta, kBatteryMin, kBatteryMax);
}

void adjustTemperatureByStep(int delta) {
    gTemperatureC = clampInt(gTemperatureC + delta, kTemperatureMin, kTemperatureMax);
}

void processManualInputFromSerial(void) {
    while (Serial.available() > 0) {
        const char key = static_cast<char>(Serial.read());
        switch (key) {
            case 'w':
            case 'W':
                adjustBatteryByStep(+5);
                Serial.printf("[INPUT] Bateria +5 -> %d%%\n", gBatteryLevel);
                break;
            case 's':
            case 'S':
                adjustBatteryByStep(-5);
                Serial.printf("[INPUT] Bateria -5 -> %d%%\n", gBatteryLevel);
                break;
            case 'e':
            case 'E':
                adjustTemperatureByStep(+2);
                Serial.printf("[INPUT] Temperatura +2 -> %d C\n", gTemperatureC);
                break;
            case 'd':
            case 'D':
                adjustTemperatureByStep(-2);
                Serial.printf("[INPUT] Temperatura -2 -> %d C\n", gTemperatureC);
                break;
            case '\n':
            case '\r':
                break;
            default:
                Serial.println("[INPUT] Tecla no valida. Usa w/s (bateria), e/d (temperatura).");
                break;
        }
    }
}