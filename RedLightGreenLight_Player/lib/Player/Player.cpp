#include "Player.h"
//#define DEBUG

const char* predefinedMacs[] = {
        "AC:15:18:4B:46:58",//Speaker
        "AC:15:18:4B:46:00",//written 2 on it
        "AC:15:18:4B:45:E4",//written 3 on it
        "AC:15:18:4B:45:F0",//written 4 on it
        "AC:15:18:4B:46:50",
    };

const int predefinedIds[] = {
    1, 2, 3, 4, 5
};

const int numPlayers = sizeof(predefinedMacs) / sizeof(predefinedMacs[0]);

Player::Player() : _status(IDLE) {

}

void Player::begin() {
    mpu.begin();
    assignIdFromMac();
}

void Player::setStatus(PlayerStatus status) {
    _status = status;
}

PlayerStatus Player::getStatus() {
    return _status;
}

int Player::getId() {
    return _id;
}

bool Player::movedDuringRedLight(int threshold) {
    // Ensure threshold is within an acceptable range
    if (threshold < 1 || threshold > 9) {
        threshold = 8;
    }

    // Check for movement using MPU6050 sensor
    return mpu.isMovementDetected(threshold);
}

void Player::assignIdFromMac() {
    uint8_t mac[6];
    WiFi.macAddress(mac);

    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    #ifdef DEBUG
    Serial.print("Retrieved MAC address: ");
    Serial.println(macStr);
    #endif
    for (int i = 0; i < numPlayers; ++i) {
        #ifdef DEBUG
        Serial.println(predefinedMacs[i]);
        #endif
        if (strcmp(macStr, predefinedMacs[i]) == 0) {
            _id = predefinedIds[i];
            #ifdef DEBUG
            Serial.print("Assigned ID ");
            Serial.print(_id);
            Serial.print(" for MAC address ");
            Serial.println(macStr);
            #endif
            return;
        }
    }

    // Default ID if MAC address is not found in the list
    _id = 7;
    #ifdef DEBUG
    Serial.print("Assigned default ID ");
    Serial.print(_id);
    Serial.print(" for MAC address ");
    Serial.println(macStr);
    #endif
}
