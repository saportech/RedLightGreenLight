#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <FastLED.h>
#include <Game.h>
#include <Player.h>

#define NUM_LEDS 6
#define DATA_PIN 27

enum SOUND {
    MOVED_SOUND,
    READY_SOUND,
    MISSION_ACCOMPLISHED_SOUND,
};

class UI {
public:
    UI();
    void setup();
    void updateReactions(int gameState, int playerStatus);
    void playSound(SOUND sound);
    void vibrateMotor();
    void setVolume(int volume);
    void setBrightness(uint8_t brightness);
    void resetVibrateFlag();

private:
    void updateLEDs(int gameState, int playerStatus);
    CRGB leds[NUM_LEDS];
    int vibrationMotorPin1 = 12;
    int vibrationMotorPin2 = 14;
    uint8_t ledBrightness = 30;
    void executeCMD(byte CMD, byte Par1, byte Par2);
    bool motorActivated = false;
};

#endif // UI_H
