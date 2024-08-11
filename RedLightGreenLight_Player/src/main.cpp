#include <Arduino.h>
#include <DFPlayer.h>
#include "Game.h"
#include "Communication.h"
#include "Player.h"
#include <FastLED.h>

#define NUM_LEDS 6
#define DATA_PIN 27

CRGB leds[NUM_LEDS];

Game game;
Communication comm;
Player player;
DFPlayer dfplayer;
Communication::Msg message;

unsigned long lastMillis = millis();

void playerStateMachine(int playerId);

void setup() {
    Serial.begin(115200);
    Serial.println("Red Light Green Light Player Unit");
    player.begin();
    Serial.println("Player ID: " + String(player.getId()));
    comm.begin();

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

void loop() {
    if (millis() - lastMillis > 1000) {
        Serial.print("Message sent to Brain Unit: ");
        Serial.print("Player ID: "); Serial.print(player.getId()); Serial.print(", ");
        Serial.print("Sensitivity: "); Serial.print(game.getSensitivity()); Serial.print(", ");
        Serial.print("Game State: "); Serial.print(game.getState()); Serial.print(", ");
        Serial.print("Player Status: "); Serial.println(player.getStatus());
        
        comm.sendMessage(player.getId(), game.getSensitivity(), game.getState(), player.getStatus());
        lastMillis = millis();
    }
}

void ledsTest() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(500);
        leds[i] = CRGB::Black;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Green;
        FastLED.show();
        delay(500);
        leds[i] = CRGB::Black;
    }
}

void playerStateMachine(int playerId) {
    static int state = 0;
    static unsigned int lastMillis = 0;

    enum RED_GREEN_STATE_TYPE {
        COMMUNICATION_SETUP,
        START,
        WAIT_FOR_RED_LIGHT,
        CHECK_IF_MOVED_DURING_RED_LIGHT,
        STATE_GAMEOVER
    };

    switch (state) {
        case COMMUNICATION_SETUP:
            Serial.println("Communication established");
            state++;
            break;
        case START:
            if (comm.receiveData() == MessageType::GAME) {
                message = comm.getMsg();
                if (message.id == player.getId() && message.game_state == GREEN) {
                    game.setSensitivity(message.sensitivity);
                    game.setState(GREEN);
                    Serial.println("Game started");
                    Serial.println("Green light");
                    dfplayer.vibrateMotor();
                    state = WAIT_FOR_RED_LIGHT;
                    break;
                } else if (message.id == player.getId() && message.game_state == GAME_OVER) {
                    Serial.println("Game over");
                    state = STATE_GAMEOVER;
                }
            }
            break;
        case WAIT_FOR_RED_LIGHT:
            if (comm.receiveData() == MessageType::GAME) {
                message = comm.getMsg();
                if (message.id == player.getId() && message.game_state == RED) {
                    game.setSensitivity(message.sensitivity);
                    game.setState(RED);
                    Serial.println("Red light");
                    state++;
                } else if (message.game_state == GAME_OVER) {
                    Serial.println("Game over");
                    state = STATE_GAMEOVER;
                } else if (message.id == player.getId() && message.player_status == CROSSED_FINISH_LINE) {
                    Serial.println("You crossed the finish line!");
                    state++;
                }
            }
            break;
        case CHECK_IF_MOVED_DURING_RED_LIGHT:
            if (player.movedDuringRedLight(game.getSensitivity())) {
                Serial.println("Player moved during red light");
                comm.sendMessage(player.getId(), 0, game.getState(), MOVED);
                state = STATE_GAMEOVER;
            }
            if (comm.receiveData() == MessageType::GAME) {
                message = comm.getMsg();
                if (message.id == player.getId() && message.game_state == GREEN) {
                    Serial.println("Green light");
                    state = WAIT_FOR_RED_LIGHT;
                } else if (message.id == player.getId() && message.game_state == GAME_OVER) {
                    Serial.println("Game over");
                    state = STATE_GAMEOVER;
                }
            }
            break;
        case STATE_GAMEOVER:
            state = COMMUNICATION_SETUP;
            break;
    }
}
