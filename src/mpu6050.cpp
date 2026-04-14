#include "mpu6050.h"
#include <math.h>

// Registros del MPU6050
#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_GYRO_XOUT_H   0x43

// Configuración inicial
#define MPU6050_ACCEL_SCALE 16384.0f  // ±2g
#define MPU6050_GYRO_SCALE 131.0f     // ±250°/s

MPU6050::MPU6050(uint8_t address) : _address(address) {}

bool MPU6050::begin() {
    Wire.begin(21, 22);  // SDA=GPIO21, SCL=GPIO22
    Wire.setClock(400000);
    
    // Wake up MPU6050 (clear sleep bit)
    writeRegister(MPU6050_REG_PWR_MGMT_1, 0x00);
    delay(10);
    
    _lastTime = micros();
    return true;
}

void MPU6050::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t MPU6050::readRegister(uint8_t reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_address, (uint8_t)1);
    return Wire.read();
}

void MPU6050::readRegisters(uint8_t reg, uint8_t *buffer, size_t len) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_address, (uint8_t)len);
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
}

void MPU6050::readAccel(float &x, float &y, float &z) {
    uint8_t buffer[6];
    readRegisters(MPU6050_REG_ACCEL_XOUT_H, buffer, 6);
    
    int16_t accelX = (buffer[0] << 8) | buffer[1];
    int16_t accelY = (buffer[2] << 8) | buffer[3];
    int16_t accelZ = (buffer[4] << 8) | buffer[5];
    
    x = accelX / MPU6050_ACCEL_SCALE;
    y = accelY / MPU6050_ACCEL_SCALE;
    z = accelZ / MPU6050_ACCEL_SCALE;
}

void MPU6050::readGyro(float &x, float &y, float &z) {
    uint8_t buffer[6];
    readRegisters(MPU6050_REG_GYRO_XOUT_H, buffer, 6);
    
    int16_t gyroX = (buffer[0] << 8) | buffer[1];
    int16_t gyroY = (buffer[2] << 8) | buffer[3];
    int16_t gyroZ = (buffer[4] << 8) | buffer[5];
    
    x = gyroX / MPU6050_GYRO_SCALE;
    y = gyroY / MPU6050_GYRO_SCALE;
    z = gyroZ / MPU6050_GYRO_SCALE;
}

float MPU6050::getYawAngle() {
    unsigned long now = micros();
    float dt = (now - _lastTime) / 1000000.0f;  // convertir a segundos
    _lastTime = now;
    
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    
    readAccel(accelX, accelY, accelZ);
    readGyro(gyroX, gyroY, gyroZ);
    
    // Estimar yaw del acelerómetro usando atan2(Y, X)
    float accelYaw = atan2(accelY, accelX) * 180.0f / M_PI;
    if (accelYaw < 0) accelYaw += 360.0f;
    
    // Filtro complementario: combina giroscopio + acelerómetro
    // 95% confianza en giroscopio (rápido), 5% en acelerómetro (corrige drift)
    _yawAngle = complementaryFilter(accelYaw, gyroZ, dt);
    
    // Normalizar a rango 0-360
    if (_yawAngle >= 360.0f) _yawAngle -= 360.0f;
    if (_yawAngle < 0.0f) _yawAngle += 360.0f;
    
    return _yawAngle;
}

float MPU6050::complementaryFilter(float accelAngle, float gyroRate, float dt) {
    const float alpha = 0.95f;  // 95% giroscopio, 5% acelerómetro
    
    // Giroscopio integrado
    float gyroAngle = _yawAngle + gyroRate * dt;
    
    // Normalizar gyroAngle antes de mezclar
    if (gyroAngle >= 360.0f) gyroAngle -= 360.0f;
    if (gyroAngle < 0.0f) gyroAngle += 360.0f;
    
    // Detectar salto de ±180° en acelerómetro y corregir
    float accelCorrected = accelAngle;
    if (fabs(accelCorrected - gyroAngle) > 180.0f) {
        if (accelCorrected > 180.0f) {
            accelCorrected -= 360.0f;
        } else {
            accelCorrected += 360.0f;
        }
    }
    
    return alpha * gyroAngle + (1.0f - alpha) * accelCorrected;
}

void MPU6050::resetYaw() {
    _yawAngle = 0.0f;
}
