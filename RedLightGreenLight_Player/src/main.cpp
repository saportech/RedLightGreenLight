#include <Arduino.h>
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include "UI.h"
#include <BLEDevice.h>

//#define DEBUG

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
    #ifdef DEBUG
    Serial.println("Red Light Green Light Player Unit");
    #endif

    ui.setup();

    player.begin();
    playerId = player.getId();
    
    comm.begin(playerId);

    BLEDevice::init("Player"); // Set the device name

    // Create BLE Advertising instance
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->setAppearance(0x0000); // Optional: Set appearance to a default value
    pAdvertising->setScanResponse(false); // Optional: Set scan response to false
    pAdvertising->setMinPreferred(0x06);  // Set the min preferred interval (in units of 0.625 ms)
    pAdvertising->setMaxPreferred(0x12);  // Set the max preferred interval (in units of 0.625 ms)

    // Start advertising
    BLEDevice::startAdvertising();
    //Serial.println("BLE Advertising started");
    

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
        GREEN_LIGHT,
        WAIT_BEFORE_RED_LIGHT,
        CHECK_IF_MOVED_DURING_RED_LIGHT,
        MOVED_DURING_RED_LIGHT,
        STATE_CELEBRATE_VICTORY_OR_LOSS,
        STATE_GAMEOVER
    };

    Communication::Msg message;
    comm.receiveData();
    message = comm.getMsg();
    ui.updateReactions(game.getState(), player.getStatus());

    if (message.game_state == GAME_OVER) {
        state = STATE_GAMEOVER;
    }

    switch (state) {
        case COMMUNICATION_SETUP:
            if (comm.establishedCommunication(message, playerId) ||
                (message.player_status == PLAYING && player.getStatus() != PLAYING ) ) {
                handleGamePlayerState(PRE_GAME, ESTABLISHED_COMMUNICATION, message.sensitivity);
                ui.playSound(READY_SOUND);
                #ifdef DEBUG
                Serial.println("Communication established");
                #endif
                state = START;
            }
            break;
        case START://State will be GREEN after GAME_BEGIN
            if (message.game_state == GAME_BEGIN && game.getState() != GAME_BEGIN || 
                (message.player_status == PLAYING && player.getStatus() != PLAYING)) {
                handleGamePlayerState(GAME_BEGIN, PLAYING, message.sensitivity);
                gameOverExecuted = false;
                break;
            }
            if (game.getState() == GAME_BEGIN) {
                if (message.game_state == RED) {
                    handleGamePlayerState(RED, player.getStatus(), message.sensitivity);
                    previousMillis = millis();
                    state = WAIT_BEFORE_RED_LIGHT;
                }
            }
            break;
        case GREEN_LIGHT://State is GREEN
            if (message.game_state == RED) {
                handleGamePlayerState(RED, player.getStatus(), message.sensitivity);
                previousMillis = millis();
                state = WAIT_BEFORE_RED_LIGHT;
            } else if (message.player_status == CROSSED_FINISH_LINE) {
                ui.playSound(MISSION_ACCOMPLISHED_SOUND);
                handleGamePlayerState(GREEN, CROSSED_FINISH_LINE, message.sensitivity);
                ui.resetVibrateFlag();
                previousMillis = millis();
                state = STATE_CELEBRATE_VICTORY_OR_LOSS;
            }
            break;
        case WAIT_BEFORE_RED_LIGHT://State is before RED
            if (millis() - previousMillis >= 2000) {
                state = CHECK_IF_MOVED_DURING_RED_LIGHT;
            }
            break;
        case CHECK_IF_MOVED_DURING_RED_LIGHT://State is RED
            if (player.movedDuringRedLight(game.getSensitivity()) && player.getStatus() == PLAYING) {
                ui.resetVibrateFlag();
                ui.playSound(MOVED_SOUND);
                handleGamePlayerState(GAME_OVER, MOVED, message.sensitivity);
                comm.sendMessage(playerId, 9, game.getSensitivity(), game.getState(), player.getStatus());
                previousMillis = millis();
                previousSendMillis = millis();
                state = MOVED_DURING_RED_LIGHT;
            }
            if (message.game_state == GREEN) {
                handleGamePlayerState(GREEN, player.getStatus(), message.sensitivity);
                state = GREEN_LIGHT;
            }
            break;
        case MOVED_DURING_RED_LIGHT:
            if (millis() - previousSendMillis > 3000) {
                comm.sendMessage(playerId, 9, game.getSensitivity(), game.getState(), player.getStatus());
                previousSendMillis = millis();
            }
            if (message.id_receiver == playerId && message.player_status == NOT_PLAYING) {
                //Serial.println("Player " + String(playerId) + " got the ACK from the brain");
                previousSendMillis = millis();
                state = STATE_CELEBRATE_VICTORY_OR_LOSS;
            }
            break;
        case STATE_CELEBRATE_VICTORY_OR_LOSS:
            if (millis() - previousMillis > 4000) {
                state = STATE_GAMEOVER;
            }
            break;
        case STATE_GAMEOVER:
            if (!gameOverExecuted) {
                handleGamePlayerState(GAME_OVER, NOT_PLAYING, message.sensitivity);
                gameOverExecuted = true;
            }
            state = START;
            break;
    }
}

void handleGamePlayerState(GameState newGameState, PlayerStatus newPlayerStatus, int newSensitivity) {
    game.setState(newGameState);
    player.setStatus(newPlayerStatus);
    game.setSensitivity(newSensitivity);

#ifdef DEBUG
    switch (game.getState()) {
        case GAME_BEGIN:
            Serial.print("Game begin");
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
#endif
}