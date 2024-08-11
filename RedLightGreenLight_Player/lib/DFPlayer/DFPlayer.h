#ifndef DFPlayer_h
#define DFPlayer_h

#include <Arduino.h>

#define GAME_STARTED_SOUND 1
#define GAME_OVER_SOUND 2
#define PLAYER_CROSSED_FINISH_LINE_SOUND 3

class DFPlayer {
public:
    DFPlayer();
    void playSound(int soundNumber);
    void vibrateMotor();
private:
    void executeCMD(byte CMD, byte Par1, byte Par2);
    bool isBusy();
    void setVolume(int volume);
    int vibrationMotorPin1 = 14;
    int vibrationMotorPin2 = 12;
};

#endif
