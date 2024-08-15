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

MessageType Communication::receiveData(int playerId) {
    
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        LoRa.readBytes((uint8_t*)&message, sizeof(message));
        messageReceived = true;

        if (message.id_sender == BRAIN_ID) {
            printMessageDetails(message);
            return MessageType::GAME;
        } else {
            return MessageType::UNKNOWN;
        }
    }
    messageReceived = false;
    return MessageType::UNKNOWN;
}

void Communication::printMessageDetails(const Msg& message) {
    Serial.print("Message received - ID Sender: ");
    Serial.print(message.id_sender);
    Serial.print(", ID Receiver: ");
    Serial.print(message.id_receiver);
    Serial.print(", Sensitivity: ");
    Serial.print(message.sensitivity);
    Serial.print(", Game State: ");
    Serial.print(gameStateToString(message.game_state));
    Serial.print(", Player Status: ");
    Serial.println(playerStatusToString(message.player_status));

}

const char* Communication::gameStateToString(GameState state) {
    switch (state) {
        case PRE_GAME: return "Pre-Game";
        case GAME_BEGIN: return "Game Begin";
        case RED: return "Red Light";
        case GREEN: return "Green Light";
        case GAME_OVER: return "Game Over";
        default: return "Unknown";
    }
}

const char* Communication::playerStatusToString(PlayerStatus status) {
    switch (status) {
        case IDLE: return "Idle";
        case PLAYING: return "Playing";
        case NOT_PLAYING: return "Not Playing";
        case MOVED: return "Moved During Red Light";
        case CROSSED_FINISH_LINE: return "Crossed Finish Line";
        default: return "Unknown";
    }
}

Communication::Msg Communication::getMsg() {
    return message;
}

void Communication::sendMessage(int id, int sensitivity, GameState game_state, PlayerStatus player_status) {
    Communication::Msg messageToSend;
    messageToSend.id_sender = id;
    messageToSend.id_receiver = BRAIN_ID;
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
            if (receiveData(playerId) == MessageType::ESTABLISH) {
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
            if (receiveData(playerId) == MessageType::ESTABLISH) {
                currentState = CommunicationState::Completed;
            }
            break;
        case CommunicationState::Completed:
            return true;
    }
    return false;
}
