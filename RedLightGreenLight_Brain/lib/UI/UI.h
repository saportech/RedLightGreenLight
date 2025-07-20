#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Game.h"
#include "Player.h"

// Pin definitions
#define NUM_LEDS 38  // Increased to handle the 6 additional LEDs
#define DATA_PIN 26

#define SEL0 2
#define SEL1 13
#define SEL2 14
#define SEL3 27
#define IO_IN 32

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)

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
    PLAYER_1_BUTTON_PRESSED,
    PLAYER_2_BUTTON_PRESSED,
    PLAYER_3_BUTTON_PRESSED,
    PLAYER_4_BUTTON_PRESSED,
    PLAYER_5_BUTTON_PRESSED,
    NO_BUTTON_PRESSED
};

// LED Index Definitions
enum LED_INDEX {
    PLAYER1_LED = 0,
    PLAYER2_LED,
    PLAYER3_LED,
    PLAYER4_LED,
    PLAYER5_LED,
    GAME_STATE_LED,
    TEAM_LED,
    INDIVIDUAL_LED,
    AUTOMATIC_LED,
    MANUAL_LED,
    // Additional Game State LEDs
    GAME_STATE_LED1,
    GAME_STATE_LED2,
    GAME_STATE_LED3,
    GAME_STATE_LED4,
    GAME_STATE_LED5,
    GAME_STATE_LED6,
    GAME_STATE_LED7,
    GAME_STATE_LED8,
    GAME_STATE_LED9,
    GAME_STATE_LED10,
    GAME_STATE_LED11,
    GAME_STATE_LED12,
    GAME_STATE_LED13,
    GAME_STATE_LED14,
    GAME_STATE_LED15,
    GAME_STATE_LED16,
    GAME_STATE_LED17,
    GAME_STATE_LED18,
    GAME_STATE_LED19,
    GAME_STATE_LED20,
    // Adding 6 more LEDs
    GAME_STATE_LED21,
    GAME_STATE_LED22,
    GAME_STATE_LED23,
    GAME_STATE_LED24,
    GAME_STATE_LED25,
    GAME_STATE_LED26
};

// Sound types
enum SOUND_TYPE {
    RED_LIGHT_SOUND,
    GREEN_LIGHT_SOUND,
    GAME_OVER_SOUND,
    GAME_BEGIN_SOUND,
    ALL_PLAYERS_READY_SOUND
};

#define BRIGHTNESS_SCALE 255

class UI {
public:
    UI();
    void setupPinsAndSensors();
    BUTTON_PRESSED buttonPressed();
    void updateLEDs(GameState gameState, GameMode gameMode, Player players[], int numPlayers);
    void playSound(SOUND_TYPE soundType);
    void printSensitivity(int sensitivity);
    void printMessage(String message);
private:
    Adafruit_SSD1306 display;
    CRGB leds[NUM_LEDS];
    bool buttonState[10]; // State of each button
    void selectMuxChannel(int channel);
    bool isBusy();
    void setVolume(int volume);
    void executeCMD(byte CMD, byte Par1, byte Par2);
};

#endif // UI_H
