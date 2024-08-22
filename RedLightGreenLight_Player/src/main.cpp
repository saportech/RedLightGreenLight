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
void handleGamePlayerState(GameState newGameState, PlayerStatus newPlayerStatus, int newSensitivity);

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
    static bool gameOverExecuted = false;
    static unsigned long previousMillis = 0;
    static unsigned long previousSendMillis = 0;

    enum RED_GREEN_STATE_TYPE {
        COMMUNICATION_SETUP,
        START,
        WAIT_FOR_RED_LIGHT,
        CHECK_IF_MOVED_DURING_RED_LIGHT,
        MOVED_DURING_RED_LIGHT,
        STATE_GAMEOVER
    };

    Communication::Msg message;
    message = comm.getMsg();

    if (message.game_state == GAME_OVER) {
        state = STATE_GAMEOVER;
    }

    switch (state) {
        case COMMUNICATION_SETUP:
            comm.receiveData(playerId);
            Serial.println("Communication established");
            state = START;
            break;
        case START:
            comm.receiveData(playerId);
            if (message.game_state == GAME_BEGIN) {
                handleGamePlayerState(GAME_BEGIN, PLAYING, message.sensitivity);
                handleGamePlayerState(GREEN, PLAYING, message.sensitivity);
                gameOverExecuted = false;
                state = WAIT_FOR_RED_LIGHT;
                break;
            }
            break;
        case WAIT_FOR_RED_LIGHT://State is GREEN
            comm.receiveData(playerId);
            if (message.game_state == RED) {
                handleGamePlayerState(RED, player.getStatus(), message.sensitivity);
                state = CHECK_IF_MOVED_DURING_RED_LIGHT;
            } else if (message.player_status == CROSSED_FINISH_LINE) {
                handleGamePlayerState(GREEN, CROSSED_FINISH_LINE, message.sensitivity);
                state = STATE_GAMEOVER;
            }
            break;
        case CHECK_IF_MOVED_DURING_RED_LIGHT://State is RED
            comm.receiveData(playerId);
            if (player.movedDuringRedLight(game.getSensitivity())) {
                handleGamePlayerState(GAME_OVER, NOT_PLAYING, message.sensitivity);
                previousMillis = millis();
                previousSendMillis = millis();
                state = MOVED_DURING_RED_LIGHT;
            }
            if (message.game_state == GREEN) {
                handleGamePlayerState(GREEN, PLAYING, message.sensitivity);
                state = WAIT_FOR_RED_LIGHT;
            }
            break;
        case MOVED_DURING_RED_LIGHT:
            comm.receiveData(playerId);
            if (millis() - previousSendMillis > 1000) {
                comm.sendMessage(playerId, player.getStatus());
                Serial.println("Player " + String(playerId) + " sent moved to the brain");
                Serial.println("message id_receiver: " + String(message.id_receiver) + " player_status: " + String(message.player_status));
                previousSendMillis = millis();
            }
            if (message.id_receiver == playerId && message.player_status == NOT_PLAYING) {
                Serial.println("Player " + String(playerId) + " got the ACK from the brain");
                state = STATE_GAMEOVER;
            }
            break;
        case STATE_GAMEOVER:
            comm.receiveData(playerId);
            if (!gameOverExecuted) {
                handleGamePlayerState(GAME_OVER, NOT_PLAYING, message.sensitivity);
                gameOverExecuted = true;
            }
            state = COMMUNICATION_SETUP;
            break;
    }
}

void handleGamePlayerState(GameState newGameState, PlayerStatus newPlayerStatus, int newSensitivity) {
    game.setState(newGameState);
    player.setStatus(newPlayerStatus);
    game.setSensitivity(newSensitivity);
    ui.updateReactions(game.getState(), player.getStatus());

    switch (game.getState()) {
        case GAME_BEGIN:
            Serial.print("Game started");
            break;
        case RED:
            Serial.print("Red light");
            break;
        case GREEN:
            Serial.print("Green light");
            break;
        case GAME_OVER:
            Serial.print("Game over");
            break;
    }
    switch (player.getStatus()) {
        case PLAYING:
            Serial.print(" Playing");
            break;
        case NOT_PLAYING:
            Serial.print(" Not playing");
            break;
        case MOVED:
            Serial.print(" Moved");
            break;
        case CROSSED_FINISH_LINE:
            Serial.print(" Crossed finish line");
            break;
    }

    Serial.println(" Sensitivity: " + String(game.getSensitivity()));
}