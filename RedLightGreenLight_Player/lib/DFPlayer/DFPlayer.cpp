#include "DFPlayer.h"

DFPlayer::DFPlayer() {
    Serial2.begin(9600, SERIAL_8N1, 34, 13);
    setVolume(15);
    pinMode(vibrationMotorPin1, OUTPUT);
    pinMode(vibrationMotorPin2, OUTPUT);
}

void DFPlayer::playSound(int soundNumber) {
    switch (soundNumber) {
        case 1:
            executeCMD(0x0F, 0x01, 0x01);
            break;
        case 2:
            executeCMD(0x0F, 0x01, 0x02);
            break;
        case 3:
            executeCMD(0x0F, 0x01, 0x03);
            break;
        case 4:
            executeCMD(0x0F, 0x01, 0x04);
            break;
        case 5:
            executeCMD(0x0F, 0x01, 0x05);
            break;
        default:
            Serial.println("Error in playSound()");
            break;
    }
}

void DFPlayer::executeCMD(byte CMD, byte Par1, byte Par2) {
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

bool DFPlayer::isBusy() {
    #define BUSY_PIN 25
    pinMode(BUSY_PIN, INPUT);
    int busyRead = digitalRead(BUSY_PIN);
    if (busyRead == 1) {
        Serial.println("DFPlayer not busy!");
        return false;
    }
    return true;
}

void DFPlayer::setVolume(int volume) {
    executeCMD(0x06, 0, volume);
    delay(500);
    Serial.println("Volume set to: " + String(volume));
}

void DFPlayer::vibrateMotor() {
    digitalWrite(vibrationMotorPin2, HIGH);
    delay(1000);
    digitalWrite(vibrationMotorPin2, LOW);
    delay(1000);
    digitalWrite(vibrationMotorPin2, HIGH);
    delay(1000);
    digitalWrite(vibrationMotorPin2, LOW);
}
