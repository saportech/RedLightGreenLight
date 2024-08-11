#include "Communication.h"

Communication::Communication() : messageReceived(false), currentState(CommunicationState::WaitingForEstablishMessage) {
}

void Communication::begin() {

    LoRa.setPins(SS, RST, DI0);

    if (!LoRa.begin(BAND)) {
        Serial.println("LoRa initialization failed. Check your connections.");
        while (1);
    }

    LoRa.setSyncWord(0xF3);
}

MessageType Communication::receiveData() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        LoRa.readBytes((uint8_t*)&message, sizeof(message));
        messageReceived = true;

        Serial.print("Message received - ID: ");
        Serial.print(message.id);
        Serial.print(", Sensitivity: ");
        Serial.print(message.sensitivity);
        Serial.print(", Game State: ");
        Serial.print(static_cast<int>(message.game_state));
        Serial.print(", Player Status: ");
        Serial.println(static_cast<int>(message.player_status));

        return MessageType::GAME;
    }
    messageReceived = false;
    return MessageType::UNKNOWN;
}

Communication::Msg Communication::getMsg() {
    return message;
}

void Communication::sendMessage(int id, int sensitivity, GameState game_state, PlayerStatus player_status) {
    Communication::Msg messageToSend;
    messageToSend.id = id;
    messageToSend.sensitivity = sensitivity;
    messageToSend.game_state = game_state;
    messageToSend.player_status = player_status;

    LoRa.beginPacket();
    LoRa.write((uint8_t*)&messageToSend, sizeof(messageToSend));
    LoRa.endPacket();
}

bool Communication::establishedCommunication(int playerId) {
    static unsigned long lastMillis = millis();
    
    switch (currentState) {
        case CommunicationState::WaitingForEstablishMessage:
            if (receiveData() == MessageType::ESTABLISH) {
                Serial.println("Moving to SendingEstablishMessage state,");
                currentState = CommunicationState::SendingEstablishMessage;
                lastMillis = millis();
            }
            break;
        case CommunicationState::SendingEstablishMessage:
            if (millis() - lastMillis > 2000) {
                sendMessage(playerId, 1, GameState::PRE_GAME, PlayerStatus::IDLE);
                Serial.println("Establish message sent by me, the Brain. Waiting for player to send");
                lastMillis = millis();
            }
            if (receiveData() == MessageType::ESTABLISH) {
                currentState = CommunicationState::Completed;
            }
            break;
        case CommunicationState::Completed:
            return true;
    }
    return false;
}
