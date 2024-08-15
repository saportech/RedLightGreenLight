#include "UI.h"

UI::UI() {

}

void UI::setupPinsAndSensors() {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.show();
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);
    pinMode(SEL2, OUTPUT);
    pinMode(SEL3, OUTPUT);
    pinMode(IO_IN, INPUT_PULLUP);
}

BUTTON_PRESSED UI::buttonPressed() {
    for (int i = 0; i < 10; i++) {
        selectMuxChannel(i);
        delay(1);
        int buttonState = digitalRead(IO_IN);
        if (buttonState == LOW) {
            return static_cast<BUTTON_PRESSED>(i);
        }
    }
    return NO_BUTTON_PRESSED;
}

void UI::selectMuxChannel(int channel) {
    digitalWrite(SEL0, (channel & 0x01) ? HIGH : LOW);
    digitalWrite(SEL1, (channel & 0x02) ? HIGH : LOW);
    digitalWrite(SEL2, (channel & 0x04) ? HIGH : LOW);
    digitalWrite(SEL3, (channel & 0x08) ? HIGH : LOW);
}

void UI::setLedColor(int ledIndex, CRGB color) {
    if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
        leds[ledIndex] = color;
        leds[ledIndex].nscale8(BRIGHTNESS_SCALE);  // Apply the compile-time brightness scale
    }
    FastLED.show();
}

void UI::updateLEDs(GameState gameState, Player players[], int numPlayers) {

    for (int i = 0; i < numPlayers; i++) {
        if (gameState == GAME_OVER) {
            leds[i] = CRGB::Yellow; // Turn all player LEDs yellow if the game is over
        } else {
            switch (players[i].getStatus()) {
                case PLAYING:
                    leds[i] = CRGB::Blue;
                    break;
                case NOT_PLAYING:
                    leds[i] = CRGB::Red;
                    break;
                default:
                    leds[i] = CRGB::Black; // Default color if none of the conditions are met
                    break;
            }
        }
        leds[i].nscale8(BRIGHTNESS_SCALE);  // Apply brightness scale
    }

    // Update the game state LED based on the current state of the game
    switch (gameState) {
        case GREEN:
            leds[GAME_STATE_LED] = CRGB::Green;
            break;
        case RED:
            leds[GAME_STATE_LED] = CRGB::Red;
            break;
        case GAME_OVER:
            leds[GAME_STATE_LED] = CRGB::Yellow; // Make the game state LED yellow when the game is over
            break;
        default:
            leds[GAME_STATE_LED] = CRGB::Black;
            break;
    }
    leds[GAME_STATE_LED].nscale8(BRIGHTNESS_SCALE);

    FastLED.show();
}

