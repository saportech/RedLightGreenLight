#include "Game.h"

Game::Game() {
    _state = PRE_GAME; // Initialize state as PRE_GAME by default
    _sensitivity = 8; // Initialize sensitivity as 8 by default
    _gameMode = INDIVIDUAL_AUTOMATIC; // Initialize game mode as INDIVIDUAL_AUTOMATIC by default
}

void Game::setState(GameState state) {
    _state = state;
}

GameState Game::getState() {
    return _state;
}

void Game::setSensitivity(int sensitivity) {
    // Ensure sensitivity is within the range 1-9
    if (sensitivity >= 1 && sensitivity <= 9) {
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
