#ifndef DFPlayer_h
#define DFPlayer_h

#include <Arduino.h>

class DFPlayer {
public:
    DFPlayer();
    void playSound(int soundNumber);
private:
    void executeCMD(byte CMD, byte Par1, byte Par2);
    bool isBusy();
    void setVolume(int volume);
};

#endif
