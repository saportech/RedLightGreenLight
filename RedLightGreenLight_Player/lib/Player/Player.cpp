#include "Player.h"
#include <WiFi.h>

#define BLE_SERVER_NAME "ESP32_Player"

const char* predefinedMacs[] = {
    "30:C9:22:B8:AD:7C",  // Replace with actual MAC addresses
    "30:C9:22:B8:AD:7D",
    "XX:XX:XX:XX:XX:03",
    "XX:XX:XX:XX:XX:04",
    "XX:XX:XX:XX:XX:05"
};

const int predefinedIds[] = {
    11, 12, 13, 14, 15  // Corresponding IDs for the MAC addresses
};

const int numPlayers = sizeof(predefinedMacs) / sizeof(predefinedMacs[0]);

Player::Player() : _status(NOT_PLAYING) {
    
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
    if (threshold < 1 || threshold > 10) {
        threshold = 5;
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

    // Print out the MAC address
    Serial.print("Retrieved MAC address: ");
    Serial.println(macStr);

    for (int i = 0; i < numPlayers; ++i) {
        Serial.println(predefinedMacs[i]);
        if (strcmp(macStr, predefinedMacs[i]) == 0) {
            _id = predefinedIds[i];
            Serial.print("Assigned ID ");
            Serial.print(_id);
            Serial.print(" for MAC address ");
            Serial.println(macStr);
            return;
        }
    }

    // Default ID if MAC address is not found in the list
    _id = 20;
    Serial.print("Assigned default ID ");
    Serial.print(_id);
    Serial.print(" for MAC address ");
    Serial.println(macStr);
}
