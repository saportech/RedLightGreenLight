#include "Game.h"
//Old macs
// const char* predefinedMacs[] = {
//         "ac:15:18:4b:46:5a",//Speaker
//         "ac:15:18:4b:46:02",//written 2 on it
//         "ac:15:18:4b:45:e6",//written 3 on it
//         "ac:15:18:4b:45:f2",//written 4 on it
//         "ac:15:18:4b:46:52",
//     };

//const int numPlayers = sizeof(predefinedMacs) / sizeof(predefinedMacs[0]);

Game::Game() {

}

void Game::begin() {
    _state = PRE_GAME; // Initialize state as IDLE by default
    _sensitivity = 7; // Initialize sensitivity as 5 by default
    _gameMode = INDIVIDUAL_MANUAL; // Initialize game mode as INDIVIDUAL_MANUAL by default
    // for (int i = 0; i < numPlayers; i++) {
    //     playerInRange[i] = false;
    // }

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

// bool Game::isPlayerInRange(int playerId, int playersNum) {
//     for (int i = 0; i < playersNum; i++) {
//         if (predefinedIds[i] == playerId) {
//             return playerInRange[i];
//         }
//     }
//     return false;
// }

// void Game::resetValues() {
//     for (int i = 0; i < numPlayers; i++) {
//         playerInRange[i] = false;
//     }
// }
