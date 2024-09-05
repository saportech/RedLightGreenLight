#include "Game.h"

const char* predefinedMacs[] = {
        "ac:15:18:4b:46:5a",//Speaker
        "ac:15:18:4b:46:02",//written 2 on it
        "ac:15:18:4b:45:e6",//written 3 on it
        "ac:15:18:4b:45:f2",//written 4 on it
        "ac:15:18:4b:46:52",
    };

const int predefinedIds[] = {
    1, 2, 3, 4, 5
};

const int numPlayers = sizeof(predefinedMacs) / sizeof(predefinedMacs[0]);

Game::Game() {

}

void Game::begin() {
    _state = PRE_GAME; // Initialize state as IDLE by default
    _sensitivity = 7; // Initialize sensitivity as 5 by default
    _gameMode = INDIVIDUAL_MANUAL; // Initialize game mode as INDIVIDUAL_MANUAL by default
    for (int i = 0; i < numPlayers; i++) {
        playerInRange[i] = false;
    }
    BLEDevice::init("esp");
    BLEScan *scan = BLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
}

void Game::setState(GameState state) {
    _state = state;
}

GameState Game::getState() {
    return _state;
}

void Game::setSensitivity(int sensitivity) {

    if (sensitivity < 1) {
        _sensitivity = 1;
    } else if (sensitivity > 9) {
        _sensitivity = 9;
    } else {
        _sensitivity = sensitivity;
    }
}

int Game::getSensitivity() {
    return _sensitivity;
}

void Game::setGameMode(GameMode mode) {
    _gameMode = mode;
}

GameMode Game::getGameMode() {
    return _gameMode;
}

bool Game::isPlayerInRange(int playerId, int playersNum) {
    for (int i = 0; i < playersNum; i++) {
        if (predefinedIds[i] == playerId) {
            return playerInRange[i];
        }
    }
    return false;
}

void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    String macAddress = advertisedDevice.getAddress().toString().c_str();
    //Serial.println("MAC: " + macAddress);
    for (int i = 0; i < numPlayers; i++) {
        if (macAddress == predefinedMacs[i]) {
            int rssi = advertisedDevice.getRSSI();
            //Serial.println("Player found: " + String(predefinedIds[i]));
            //Serial.println(" with RSSI: " + String(advertisedDevice.getRSSI()));
            if (rssi > -30) {
                //Serial.println("And is in range.");
                game->playerInRange[i] = true;
            } else {
                game->playerInRange[i] = false;
            }
        }
    }
}

void Game::resetValues() {
    for (int i = 0; i < numPlayers; i++) {
        playerInRange[i] = false;
    }
}

void Game::startScan() {
static unsigned long lastLoopMillis = 0;

    if (millis() - lastLoopMillis >= 1500) {
        BLEScan *scan = BLEDevice::getScan();
        scan->start(1, false);
        lastLoopMillis = millis();
    }

}