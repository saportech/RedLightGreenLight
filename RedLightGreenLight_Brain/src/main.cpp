#include <Arduino.h>
#include <DFPlayer.h> 
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include "UI.h"

#define NUM_PLAYERS 6
int playerIDs[NUM_PLAYERS] = {1, 2, 3, 4, 5, 9};
int currentPlayerIndex = 5;
int currentPlayer = 9;

Communication comm;
Player players[NUM_PLAYERS];
Game game;
UI ui;

void brainStateMachine();
bool playerCrossedFinishLine();

void sendMessageToAllPlayers(GameState state);
void sendMessageDuringRedLight(int id, GameState state, PlayerStatus playerStatus);

void setup() {

  Serial.begin(115200);
  Serial.println("Red Light Green Light Brain Unit");

  ui.setupPinsAndSensors();

  for (int i = 0; i < NUM_PLAYERS; i++) {
      players[i].begin(playerIDs[i]);
      Serial.print("Initialized player with ID: ");
      Serial.println(players[i].getId());
  }

  comm.begin();

}

void loop() {

    brainStateMachine();

}

void brainStateMachine() {
    static int state = 0;
    static unsigned long previousMillis = 0;
    Communication::Msg message;

    enum RED_GREEN_STATE_TYPE { 
        CHECK_COMMUNICATION,
        GAME_BEGINS,
        START,
        GREEN_LIGHT, 
        WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT,
        STATE_GAMEOVER
    };

    BUTTON_PRESSED pressedButton = ui.buttonPressed();

    if (pressedButton == END_GAME_PRESSED) {
        Serial.println("Game ended, sending message to players");
        game.setState(GAME_OVER);
        ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
        state = CHECK_COMMUNICATION;
    }
    
    switch (state) {
        case CHECK_COMMUNICATION:
            sendMessageToAllPlayers(game.getState());
            comm.resetMsg();
            Serial.println("Communication established, Waiting for operator to start game");
            state = GAME_BEGINS;
            break;
        case GAME_BEGINS:
            sendMessageToAllPlayers(game.getState());
            if (pressedButton == START_GAME_PRESSED) {
                Serial.println("Game started");
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setStatus(PLAYING);
                }
                game.setState(GAME_BEGIN);
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                state = START;
                previousMillis = millis();
            }
            break;
        case START:
            sendMessageToAllPlayers(game.getState());
            if (millis() -  previousMillis > 1000) {
                game.setState(GREEN);
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                state = GREEN_LIGHT;
            }
            break;
        case GREEN_LIGHT: // State is GREEN now
            sendMessageToAllPlayers(game.getState());
            if (pressedButton == RED_PRESSED) {
                Serial.println("Game turned RED");
                game.setState(RED);
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                state = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
            } else if (playerCrossedFinishLine()) {
                Serial.println("Player crossed finish line");
            }
            break;
        case WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT: // State is RED now
            sendMessageDuringRedLight(currentPlayer, game.getState(), players[currentPlayerIndex].getStatus());
            message = comm.getMsg();
            if (pressedButton == GREEN_PRESSED) {
                game.setState(GREEN);
                Serial.println("Game turned GREEN");
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                state = GREEN_LIGHT;
            }
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (message.id_sender == players[i].getId() && message.player_status == NOT_PLAYING && players[i].getStatus() == PLAYING) {
                    Serial.println("Player " + String(players[i].getId()) + " moved during red light");
                    players[i].setStatus(NOT_PLAYING);
                    ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                    currentPlayerIndex = i;
                    currentPlayer = players[i].getId();
                }
            }
            break;
        case STATE_GAMEOVER:
            sendMessageToAllPlayers(game.getState());
            Serial.println("Game over");
            ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
            state = CHECK_COMMUNICATION;
            break;
    }
}

void sendMessageToAllPlayers(GameState state) {

    comm.sendMessage(9, game.getSensitivity(), state, players[0].getStatus());

}

void sendMessageDuringRedLight(int id, GameState state, PlayerStatus playerStatus) {
    #define SEND_INTERVAL 500
    static unsigned long lastActionTime = 0;

    comm.receiveData();

    if (millis() - lastActionTime >= SEND_INTERVAL) {
        comm.sendMessage(id, game.getSensitivity(), state, playerStatus);
        lastActionTime = millis();
    }
    
}

bool playerCrossedFinishLine() {
    return false;
}
