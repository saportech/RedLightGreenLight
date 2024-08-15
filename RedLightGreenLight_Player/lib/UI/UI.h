#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <FastLED.h>
#include <Game.h>
#include <Player.h>

#define NUM_LEDS 6
#define DATA_PIN 27

#define GAME_STARTED_SOUND 1
#define GAME_OVER_SOUND 2
#define PLAYER_CROSSED_FINISH_LINE_SOUND 3

class UI {
public:
    UI();
    void setup();
    void updateReactions(int gameState, int playerStatus);
    void playSound(int soundNumber);
    void vibrateMotor();
    void setVolume(int volume);
    void setBrightness(uint8_t brightness);

private:
    void updateLEDs(int gameState, int playerStatus);
    CRGB leds[NUM_LEDS];
    int vibrationMotorPin1 = 12;
    int vibrationMotorPin2 = 14;
    uint8_t ledBrightness = 20;
    void executeCMD(byte CMD, byte Par1, byte Par2);
};

#endif // UI_H
