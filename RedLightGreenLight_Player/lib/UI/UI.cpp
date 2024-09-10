#include "UI.h"

UI::UI() {
    
}

void UI::setup() {
    
    pinMode(vibrationMotorPin1, OUTPUT);
    pinMode(vibrationMotorPin2, OUTPUT);
    digitalWrite(vibrationMotorPin1, HIGH);
    delay(200);
    digitalWrite(vibrationMotorPin1, LOW);
    digitalWrite(vibrationMotorPin2, LOW);

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.show();

    for (int i = 0; i < 5; i++) {
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(100);
        leds[i] = CRGB::Black;
        FastLED.show();
    }

    Serial2.begin(9600, SERIAL_8N1, 34, 13);
    setVolume(30);

    resetVibrateFlag();
}

void UI::updateReactions(int gameState, int playerStatus) {
    updateLEDs(gameState, playerStatus);  // Update LEDs based on game state and player status

    if (playerStatus == MOVED) {
        vibrateMotor();
    } else if (playerStatus == CROSSED_FINISH_LINE) {
        vibrateMotor();
    }
}

void UI::updateLEDs(int gameState, int playerStatus) {
    static unsigned long lastUpdateMillis = millis();

    CRGB gameColor = (gameState == GREEN) ? CRGB::Green : (gameState == GAME_BEGIN) ? CRGB::Green : (gameState == RED) ? CRGB::Red : (gameState == PRE_GAME) ? CRGB::Yellow : (gameState == GAME_OVER) ? CRGB::Pink : CRGB::Orange;
    CRGB statusColor = (playerStatus == PLAYING) ? CRGB::Blue : (playerStatus == NOT_PLAYING) ? CRGB::Red : (playerStatus == MOVED) ? CRGB::Yellow : (playerStatus == ESTABLISHED_COMMUNICATION) ? CRGB::Purple : CRGB::Orange;

    leds[0] = gameColor;
    leds[0].nscale8(ledBrightness);
    
    leds[5] = statusColor;
    leds[5].nscale8(ledBrightness);
    
    if (millis() - lastUpdateMillis > 50) {
        lastUpdateMillis = millis();
        FastLED.show();
    }
}

void UI::playSound(SOUND sound) {
    switch (sound) {
        case MOVED_SOUND:
            executeCMD(0x0F, 0x01, 0x01);
            break;
        case READY_SOUND:
            executeCMD(0x0F, 0x01, 0x02);
            break;
        case MISSION_ACCOMPLISHED_SOUND:
            executeCMD(0x0F, 0x01, 0x03);
            break;
        default:
            Serial.println("Error in playSound()");
            break;
    }
}

void UI::vibrateMotor() {
    static unsigned long startMillis = millis();
    static unsigned long lastToggleMillis = millis();
    static bool motorOn = false;

    if (!motorActivated) {
        motorActivated = true;
        startMillis = millis();
        lastToggleMillis = millis();
        motorOn = true;
        digitalWrite(vibrationMotorPin1, HIGH);  // Start vibration
    }

    if (motorActivated) {
        unsigned long currentMillis = millis();

        // if (currentMillis - lastToggleMillis >= 200) {
        //     lastToggleMillis = currentMillis;
        //     motorOn = !motorOn;
        //     digitalWrite(vibrationMotorPin1, motorOn ? HIGH : LOW);  // Toggle motor state
        // }

        if (currentMillis - startMillis >= 1000) {
            digitalWrite(vibrationMotorPin1, LOW);
            //motorOn = false;
        }
    }
}

void UI::resetVibrateFlag() {
    motorActivated = false;
}

void UI::setVolume(int volume) {
    executeCMD(0x06, 0, volume);
    delay(100);
}

void UI::executeCMD(byte CMD, byte Par1, byte Par2) {
    #define Start_Byte 0x7E
    #define Version_Byte 0xFF
    #define Command_Length 0x06
    #define End_Byte 0xEF
    #define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
    
    // Calculate the checksum (2 bytes)
    word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    
    // Build the command line
    byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                                Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte };
    
    // Send the command line to the module
    for (byte k = 0; k < 10; k++) {
        Serial2.write(Command_line[k]);
    }
}

void UI::setBrightness(uint8_t brightness) {
    ledBrightness = brightness;
}
