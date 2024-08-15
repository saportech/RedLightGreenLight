#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <FastLED.h>
#include "Game.h"
#include "Player.h"

// Pin definitions
#define NUM_LEDS 10
#define DATA_PIN 26

#define SEL0 2
#define SEL1 13
#define SEL2 14
#define SEL3 27
#define IO_IN 32

// Button pressed definitions
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
    SENSITIVITY_DOWN_PRESSED,
    NO_BUTTON_PRESSED // Indicates no button pressed
};

// LED Index Definitions
enum LED_INDEX {
    PLAYER1_LED = 0,
    PLAYER2_LED,
    PLAYER3_LED,
    PLAYER4_LED,
    PLAYER5_LED,
    GAME_STATE_LED,
    AUTOMATIC_LED,
    MANUAL_LED,
    TEAM_LED,
    INDIVIDUAL_LED
};

#define BRIGHTNESS_SCALE 15

class UI {
public:
    UI();
    void setupPinsAndSensors();
    BUTTON_PRESSED buttonPressed();
    void setLedColor(int ledIndex, CRGB color);
    void updateLEDs(GameState gameState, Player players[], int numPlayers);

private:
    CRGB leds[NUM_LEDS];
    bool buttonState[10]; // State of each button
    void selectMuxChannel(int channel);
};

#endif // UI_H

