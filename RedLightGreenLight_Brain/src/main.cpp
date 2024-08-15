#include <Arduino.h>
#include <DFPlayer.h> 
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include "UI.h"

#define NUM_PLAYERS 5
int playerIDs[NUM_PLAYERS] = {11, 12, 13, 14, 15};

Communication comm;
Player players[NUM_PLAYERS];
Game game;
UI ui;

void brainStateMachine();
bool playerCrossedFinishLine();
void sendMessageToAllPlayers(GameState state);

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
  
    enum RED_GREEN_STATE_TYPE { 
        CHECK_COMMUNICATION,
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
        sendMessageToAllPlayers(game.getState());
        state = CHECK_COMMUNICATION;
    }
    switch (state) {
        case CHECK_COMMUNICATION:
            Serial.println("Communication established");
            Serial.println("Waiting for operator to start game");
            state = START;
            break;
        case START:
            if (pressedButton == START_GAME_PRESSED) {
                Serial.println("Game turned GREEN, Sending message to players");
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setStatus(PLAYING);
                }
                game.setState(GAME_BEGIN);
                sendMessageToAllPlayers(game.getState());
                game.setState(GREEN);
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                sendMessageToAllPlayers(game.getState());
                state = GREEN_LIGHT;
            }
            break;
        case GREEN_LIGHT: // State is GREEN now
            if (pressedButton == RED_PRESSED) {
                Serial.println("Game turned RED");
                game.setState(RED);
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                sendMessageToAllPlayers(game.getState());
                state = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
            } else if (playerCrossedFinishLine()) {
                Serial.println("Player crossed finish line");
            }
            break;
        case WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT: // State is RED now
            if (pressedButton == GREEN_PRESSED) {
                game.setState(GREEN);
                Serial.println("Game turned GREEN");
                ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                sendMessageToAllPlayers(game.getState());
                state = GREEN_LIGHT;
            } else if (comm.receiveData() == MessageType::GAME) {
                Communication::Msg message = comm.getMsg();
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    if (message.id_sender == players[i].getId() && message.player_status == MOVED) {
                        Serial.println("Player " + String(players[i].getId()) + " moved during red light");
                        players[i].setStatus(NOT_PLAYING);
                        ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
                        //comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
                    }
                }
            }
            break;
        case STATE_GAMEOVER:
            Serial.println("Game over");
            ui.updateLEDs(game.getState(), players, NUM_PLAYERS);
            state = CHECK_COMMUNICATION;
            break;
    }
}

void sendMessageToAllPlayers(GameState state) {
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            comm.sendMessage(players[i].getId(), game.getSensitivity(), state, players[i].getStatus());
        }
    }
    Serial.println("Message sent to all players");
}

bool playerCrossedFinishLine() {
    return false;
}
