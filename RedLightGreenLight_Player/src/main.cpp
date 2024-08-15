#include <Arduino.h>
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include "UI.h"

Game game;
Communication comm;
Player player;
UI ui;

unsigned long lastMillis = millis();
int playerId;

void playerStateMachine();
void handleGamePlayerState(GameState newGameState, PlayerStatus newPlayerStatus);

void setup() {
    Serial.begin(115200);
    Serial.println("Red Light Green Light Player Unit");
    player.begin();
    
    playerId = player.getId();
    Serial.println("Player ID: " + String(playerId));
    
    comm.begin();
    ui.setup();
}

void loop() {
    playerStateMachine();
}

void playerStateMachine() {
    static int state = 0;

    enum RED_GREEN_STATE_TYPE {
        COMMUNICATION_SETUP,
        START,
        WAIT_FOR_RED_LIGHT,
        CHECK_IF_MOVED_DURING_RED_LIGHT,
        STATE_GAMEOVER
    };

    Communication::Msg message;

    switch (state) {
        case COMMUNICATION_SETUP:
            Serial.println("Communication established");
            state = START;
            break;
        case START:
            if (comm.receiveData(playerId) == MessageType::GAME) {
                message = comm.getMsg();
                if (message.game_state == GAME_BEGIN) {
                    handleGamePlayerState(GAME_BEGIN, PLAYING);
                    handleGamePlayerState(GREEN, PLAYING);
                    state = WAIT_FOR_RED_LIGHT;
                    break;
                } else if (message.game_state == GAME_OVER) {
                    handleGamePlayerState(GAME_OVER, NOT_PLAYING);
                    state = STATE_GAMEOVER;
                }
            }
            break;
        case WAIT_FOR_RED_LIGHT://State is GREEN
            if (comm.receiveData(playerId) == MessageType::GAME) {
                message = comm.getMsg();
                if (message.game_state == RED) {
                    handleGamePlayerState(RED, player.getStatus());
                    state = CHECK_IF_MOVED_DURING_RED_LIGHT;
                } else if (message.game_state == GAME_OVER) {
                    handleGamePlayerState(GAME_OVER, NOT_PLAYING);
                    state = STATE_GAMEOVER;
                } else if (message.player_status == CROSSED_FINISH_LINE) {
                    handleGamePlayerState(GREEN, CROSSED_FINISH_LINE);
                    state = STATE_GAMEOVER;
                }
            }
            break;
        case CHECK_IF_MOVED_DURING_RED_LIGHT://State is RED
            if (player.movedDuringRedLight(game.getSensitivity())) {
                handleGamePlayerState(GREEN, MOVED);
                comm.sendMessage(playerId, game.getSensitivity(), game.getState(), player.getStatus());
                state = STATE_GAMEOVER;
            }
            if (comm.receiveData(playerId) == MessageType::GAME) {
                message = comm.getMsg();
                if (message.game_state == GREEN) {
                    handleGamePlayerState(GREEN, PLAYING);
                    state = WAIT_FOR_RED_LIGHT;
                } else if (message.game_state == GAME_OVER) {
                    handleGamePlayerState(GAME_OVER, NOT_PLAYING);
                    state = STATE_GAMEOVER;
                }
            }
            break;
        case STATE_GAMEOVER:
            state = COMMUNICATION_SETUP;
            break;
    }
}

void handleGamePlayerState(GameState newGameState, PlayerStatus newPlayerStatus) {
    game.setState(newGameState);
    player.setStatus(newPlayerStatus);
    ui.updateReactions(game.getState(), player.getStatus());

    switch (game.getState()) {
        case GAME_BEGIN:
            Serial.println("Game started");
            break;
        case RED:
            Serial.println("Red light");
            break;
        case GREEN:
            Serial.println("Green light");
            break;
        case GAME_OVER:
            Serial.println("Game over");
            break;
    }
    switch (player.getStatus()) {
        case PLAYING:
            Serial.println("Playing");
            break;
        case NOT_PLAYING:
            Serial.println("Not playing");
            break;
        case MOVED:
            Serial.println("Moved");
            break;
        case CROSSED_FINISH_LINE:
            Serial.println("Crossed finish line");
            break;
    }
}