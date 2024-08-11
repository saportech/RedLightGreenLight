#include <Arduino.h>
#include <DFPlayer.h> 
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include <FastLED.h>

#define NUM_LEDS 10
#define DATA_PIN 26

CRGB leds[NUM_LEDS];

#define SEL0 2
#define SEL1 13
#define SEL2 14
#define SEL3 27

#define IO_IN 32

enum BUTTON_PRESSED { 
                    START_GAME_PRESSED,
                    END_GAME_PRESSED, 
                    RED_PRESSED,
                    GREEN_PRESSED,
                    AUTO_MODE_PRESSED,
                    MANUAL_MODE_PRESSED,
                    INDIVIDUAL_MODE_PRESSED,
                    TEAM_MODE_PRESSED,
                    SENSITIVITY_UP_PRESSED,
                    SENSITIVITY_DOWN_PRESSED
                  };

bool playerCrossedFinishLineBool = false;
unsigned long lastMillis = millis();
bool buttonState[10];

#define NUM_PLAYERS 5
int playerIDs[NUM_PLAYERS] = {11, 12, 13, 14, 15};

Communication comm;
Player players[NUM_PLAYERS];
Game game;

void brainStateMachine();
void pinsSensorsSetup();
void selectMuxChannel(int channel);
bool playerCrossedFinishLine();
int buttonPressed();

void setup() {

  Serial.begin(115200);
  Serial.println("Red Light Green Light Brain Unit");

  pinsSensorsSetup();

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
  static unsigned int lastMillis = 0;
  
  enum RED_GREEN_STATE_TYPE { 
                    CHECK_COMMUNICATION,
                    START,  
                    GREEN_LIGHT, 
                    WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT,
                    STATE_GAMEOVER
                  };

  if (buttonPressed() == END_GAME_PRESSED) {
    Serial.println("Game ended, sending message to players");
    game.setState(GAME_OVER);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].setStatus(NOT_PLAYING);
        comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
    }
    state = CHECK_COMMUNICATION;
    delay(1000);
  }

  switch ( state ) {
    case CHECK_COMMUNICATION:
      Serial.println("Communication established");
      Serial.println("Waiting for operator to start game");
      state++;
      break;
    case START:
      if (buttonPressed() == START_GAME_PRESSED) {
        Serial.println("Game turned GREEN, Sending message to players");
        game.setState(GREEN);
        for (int i = 0; i < NUM_PLAYERS; i++) {
            players[i].setStatus(PLAYING);
            comm.sendMessage(players[i].getId(), game.getSensitivity() , game.getState(), players[i].getStatus());
        }
        state = GREEN_LIGHT;
        delay(500);
      }
      break;
    case GREEN_LIGHT: // State is GREEN now
      if (buttonPressed() == RED_PRESSED) {
        Serial.println("Game turned RED");
        game.setState(RED);
        for (int i = 0; i < NUM_PLAYERS; i++) {
            comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
        }
        state = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
        delay(500);
      } else if (playerCrossedFinishLine()) {
        Serial.println("Player crossed finish line, GAME OVER");
        for (int i = 0; i < NUM_PLAYERS; i++) {
            players[i].setStatus(NOT_PLAYING);
            comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
        }
        state = STATE_GAMEOVER;
      }
      break;
    case WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT: // State is RED now
      if (buttonPressed() == GREEN_PRESSED) {
        game.setState(GREEN);
        for (int i = 0; i < NUM_PLAYERS; i++) {
            comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
        }
        state = GREEN_LIGHT;
        delay(500);
      } else if (comm.receiveData() == MessageType::GAME) {
        Communication::Msg message = comm.getMsg();
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (message.id == players[i].getId() && message.player_status == MOVED) {
                Serial.println("Player " + String(players[i].getId()) + "moved during red light");
                players[i].setStatus(NOT_PLAYING);
                comm.sendMessage(players[i].getId(), game.getSensitivity(), game.getState(), players[i].getStatus());
            }
        }
      }
      break;
    case STATE_GAMEOVER:
      Serial.println("Game over");
      state = CHECK_COMMUNICATION;
      break;
  }
}

bool playerCrossedFinishLine() {
  return false;
}

int buttonPressed() {
  for (int i = 0; i < 10; i++) {
    selectMuxChannel(i);
    delay(2);
    buttonState[i] = digitalRead(IO_IN);

    if (buttonState[i] == LOW) {
      return i;
    }
  }
  return -1; // Return -1 if no button is pressed
}

void pinsSensorsSetup() {
  // Set MUX control pins as outputs
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);
  pinMode(SEL2, OUTPUT);
  pinMode(SEL3, OUTPUT);

  // Set the MUX input pin as an input
  pinMode(IO_IN, INPUT_PULLUP);

  // Set all control pins to low initially
  digitalWrite(SEL0, LOW);
  digitalWrite(SEL1, LOW);
  digitalWrite(SEL2, LOW);
  digitalWrite(SEL3, LOW);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void selectMuxChannel(int channel) {
  digitalWrite(SEL0, (channel & 0x01) ? HIGH : LOW);
  digitalWrite(SEL1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(SEL2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(SEL3, (channel & 0x08) ? HIGH : LOW);
}
