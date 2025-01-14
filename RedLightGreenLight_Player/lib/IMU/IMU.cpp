#include "IMU.h"

bool IMU::begin() {
    int ret = _imu.begin();
    if (ret != 0) {
        Serial.print("ICM42670 initialization failed: ");
        Serial.println(ret);
        while (1);
    } else {
        Serial.println("ICM42670 initialization successful.");
    }
    _imu.startAccel(100, 16);
    _imu.startGyro(100, 2000);
    delay(100);
    return true;
}

void IMU::readSensorData(float* ax, float* ay, float* az, float* gx, float* gy, float* gz) {
    inv_imu_sensor_event_t imu_event;
    _imu.getDataFromRegisters(imu_event);

    *ax = imu_event.accel[0];
    *ay = imu_event.accel[1];
    *az = imu_event.accel[2];
    *gx = imu_event.gyro[0];
    *gy = imu_event.gyro[1];
    *gz = imu_event.gyro[2];
}

bool IMU::isMovementDetected(int userThreshold) {
    //4000 almost every movement, 10000 medium, 15000 almost impossible to reach
    int threshold = map(userThreshold, 10, 1, 4000, 15000);
    // Track time using millis()
    static unsigned long lastSampleTime = 0;
    unsigned long currentTime = millis();
    #define SAMPLE_TIME 50

    if (currentTime - lastSampleTime >= SAMPLE_TIME) {
        float ax, ay, az, gx, gy, gz;
        readSensorData(&ax, &ay, &az, &gx, &gy, &gz);

        // Calculate the absolute sum of sensor values
        int absoluteSum = abs(ax) + abs(ay) + abs(az) + abs(gx) + abs(gy) + abs(gz);

        lastSampleTime = currentTime;

        return (absoluteSum > threshold);
    }

    return false;
}

