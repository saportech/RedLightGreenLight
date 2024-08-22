#include "MPU6050_ESP32.h"

#define MPU6050_ADDRESS     0x68

MPU6050_ESP32::MPU6050_ESP32() {
    
}

bool MPU6050_ESP32::begin() {
  Wire.begin();
  writeRegister(MPU6050_ADDRESS, 0x6B, 0x00); // Wake up the MPU6050
  return testConnection();
}

void MPU6050_ESP32::calibrateGyroscope() {
  // Implement gyroscope calibration here if needed
}

bool MPU6050_ESP32::testConnection() {
  uint8_t data;
  readRegisters(MPU6050_ADDRESS, 0x75, 1, &data);
  return data == 0x68; // MPU6050 default address
}

void MPU6050_ESP32::readSensorData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
  uint8_t buffer[14];
  readRegisters(MPU6050_ADDRESS, 0x3B, 14, buffer);
  *ax = ((int16_t)buffer[0] << 8) | buffer[1];
  *ay = ((int16_t)buffer[2] << 8) | buffer[3];
  *az = ((int16_t)buffer[4] << 8) | buffer[5];
  *gx = ((int16_t)buffer[8] << 8) | buffer[9];
  *gy = ((int16_t)buffer[10] << 8) | buffer[11];
  *gz = ((int16_t)buffer[12] << 8) | buffer[13];
}

bool MPU6050_ESP32::isMovementDetected(int userThreshold) {
  // Map the user-provided threshold value to the desired range (7000-100000)
  int threshold = map(userThreshold, 9, 1, 7000, 100000);
  
  // Track time using millis()
  static unsigned long lastSampleTime = 0;
  unsigned long currentTime = millis();
  #define SAMPLE_TIME 50

  if (currentTime - lastSampleTime >= SAMPLE_TIME) {
    int16_t ax, ay, az, gx, gy, gz;
    readSensorData(&ax, &ay, &az, &gx, &gy, &gz);
    
    int absoluteSum = abs(ax) + abs(ay) + abs(az) + abs(gx) + abs(gy) + abs(gz);
  
    lastSampleTime = currentTime;

    //Serial.println(absoluteSum);
    
    return (absoluteSum > threshold);
  }
  
  return false;
}

void MPU6050_ESP32::writeRegister(uint8_t address, uint8_t reg, uint8_t data) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void MPU6050_ESP32::readRegisters(uint8_t address, uint8_t reg, uint8_t count, uint8_t* dest) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(address, count);
  for (int i = 0; i < count; i++) {
    dest[i] = Wire.read();
  }
}
