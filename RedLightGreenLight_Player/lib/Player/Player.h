#ifndef Player_h
#define Player_h

#include <Arduino.h>
#include "MPU6050_ESP32.h"
#include <WiFi.h>

enum PlayerStatus {
    IDLE,
    ESTABLISHED_COMMUNICATION,
    PLAYING,
    NOT_PLAYING,
    MOVED,
    CROSSED_FINISH_LINE
};



class Player {
public:
    Player();
    void begin();
    void setStatus(PlayerStatus status);
    PlayerStatus getStatus();
    bool movedDuringRedLight(int threshold);
    int getId();
private:
    int _id;
    PlayerStatus _status;
    MPU6050_ESP32 mpu;

    void assignIdFromMac();
};

#endif
