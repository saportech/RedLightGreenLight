#include "Game.h"

Game::Game() {
    _state = PRE_GAME; // Initialize state as IDLE by default
    _sensitivity = 8; // Initialize sensitivity as 5 by default
    _gameMode = INDIVIDUAL_MANUAL; // Initialize game mode as INDIVIDUAL_AUTOMATIC by default
}

void Game::setState(GameState state) {
    _state = state;
}

GameState Game::getState() {
    return _state;
}

void Game::setSensitivity(int sensitivity) {
    // Ensure sensitivity is within the range 1-10
    if (sensitivity >= 1 && sensitivity <= 10) {
        _sensitivity = sensitivity;
    } else {
        Serial.println("Invalid sensitivity value. Setting to default (5).");
        _sensitivity = 5;
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
