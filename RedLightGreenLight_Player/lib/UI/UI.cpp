#include "UI.h"

UI::UI() {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void UI::setup() {
    Serial2.begin(9600, SERIAL_8N1, 34, 13);
    pinMode(vibrationMotorPin1, OUTPUT);
    pinMode(vibrationMotorPin2, OUTPUT);
    digitalWrite(vibrationMotorPin1, LOW);
    digitalWrite(vibrationMotorPin2, LOW);

    FastLED.clear();
    FastLED.show();

    //setVolume(15);
}

void UI::updateReactions(int gameState, int playerStatus) {
    updateLEDs(gameState, playerStatus);  // Update LEDs based on game state and player status

    if (gameState == GAME_OVER) {
        playSound(GAME_OVER_SOUND);
        vibrateMotor();
    } else if (gameState == GREEN) {
        playSound(GAME_STARTED_SOUND);
    } else if (playerStatus == CROSSED_FINISH_LINE) {
        playSound(PLAYER_CROSSED_FINISH_LINE_SOUND);
        vibrateMotor();
    }
}

void UI::updateLEDs(int gameState, int playerStatus) {
    CRGB gameColor = (gameState == GREEN) ? CRGB::Green : (gameState == RED) ? CRGB::Red : CRGB::Yellow;
    CRGB statusColor = (playerStatus == PLAYING) ? CRGB::Blue : (playerStatus == NOT_PLAYING) ? CRGB::Red : (playerStatus == MOVED) ? CRGB::Orange : CRGB::Purple;

    for (int i = 0; i < 3; i++) {
        leds[i] = gameColor;
        leds[i].nscale8(ledBrightness);
    }
    for (int i = 3; i < NUM_LEDS; i++) {
        leds[i] = statusColor;
        leds[i].nscale8(ledBrightness);
    }
    FastLED.show();
}

void UI::playSound(int soundNumber) {
    executeCMD(0x0F, 0x01, soundNumber);
}

void UI::vibrateMotor() {
    // digitalWrite(vibrationMotorPin, HIGH);
    // delay(1000);
    // digitalWrite(vibrationMotorPin, LOW);
}

void UI::setVolume(int volume) {
    executeCMD(0x06, 0, volume);
}

void UI::executeCMD(byte CMD, byte Par1, byte Par2) {
    byte Command_line[10] = { 0x7E, 0xFF, 0x06, CMD, 0x00, Par1, Par2, 0xEF };
    for (byte k = 0; k < 10; k++) {
        Serial2.write(Command_line[k]);
    }
}

void UI::setBrightness(uint8_t brightness) {
    ledBrightness = brightness;
}
