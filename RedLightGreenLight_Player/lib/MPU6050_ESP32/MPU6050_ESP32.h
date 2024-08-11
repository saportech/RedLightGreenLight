#ifndef MPU6050_ESP32_H
#define MPU6050_ESP32_H

#include <Wire.h>
#include <Arduino.h>

class MPU6050_ESP32 {
  public:
    MPU6050_ESP32();
    bool begin();
    void calibrateGyroscope();
    bool testConnection();
    void readSensorData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
    bool isMovementDetected(int threshold);
    
  private:
    void writeRegister(uint8_t address, uint8_t reg, uint8_t data);
    void readRegisters(uint8_t address, uint8_t reg, uint8_t count, uint8_t* dest);
};

#endif
