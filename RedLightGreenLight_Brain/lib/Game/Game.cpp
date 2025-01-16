#include "Game.h"

Game::Game() {

}

void Game::begin() {
    _state = PRE_GAME; // Initialize state as IDLE by default
    _sensitivity = 7; // Initialize sensitivity as 5 by default
    _gameMode = INDIVIDUAL_MANUAL; // Initialize game mode as INDIVIDUAL_MANUAL by default

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