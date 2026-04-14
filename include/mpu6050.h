#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>
#include <Wire.h>

class MPU6050 {
public:
    MPU6050(uint8_t address = 0x68);
    
    bool begin();
    void readAccel(float &x, float &y, float &z);
    void readGyro(float &x, float &y, float &z);
    float getYawAngle();  // Ángulo en el eje Z con filtro complementario
    void resetYaw();      // Resetear el ángulo a 0
    
private:
    float complementaryFilter(float accelAngle, float gyroRate, float dt);
    
private:
    uint8_t _address;
    float _yawAngle = 0.0f;
    unsigned long _lastTime = 0;
    
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    void readRegisters(uint8_t reg, uint8_t *buffer, size_t len);
};

#endif // MPU6050_H
