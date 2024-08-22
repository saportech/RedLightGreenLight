#include "Communication.h"

Communication::Communication() : messageReceived(false), currentState(CommunicationState::WaitingForEstablishMessage) {
}

void Communication::begin() {
    LoRa.setPins(SS, RST, DI0);

    if (!LoRa.begin(BAND)) {
        Serial.println("LoRa initialization failed. Check your connections.");
        while (1);
    }

    LoRa.setSpreadingFactor(10);
}

void Communication::receiveData() {
    int packetSize = LoRa.parsePacket();
    String incoming = "";

    if (packetSize) {

        while (LoRa.available()) {
            incoming += (char)LoRa.read();
        }

        if (incoming.endsWith(" E")) {
            incoming.remove(incoming.length() - 2);
        }

        parseMessage(incoming);
    }
}

void Communication::parseMessage(const String& incoming) {
    int id = 0;
    PlayerStatus player_status = PlayerStatus::NOT_PLAYING;

    // Process the message string to extract values
    int index = 0;
    String token = "";
    int part = 0;
    String LoRaMessage = incoming;

    while ((index = LoRaMessage.indexOf(' ')) != -1) {
        token = LoRaMessage.substring(0, index);
        LoRaMessage = LoRaMessage.substring(index + 1);

        switch (part) {
            case 0:
                id = token.toInt();
                break;
            case 1:
                player_status = static_cast<PlayerStatus>(token.toInt());
                break;
            default:
                break;
        }
        part++;
    }

    if (LoRaMessage.length() > 0) {
        if (part == 0) {
            id = LoRaMessage.toInt();
        } else if (part == 1) {
            player_status = static_cast<PlayerStatus>(LoRaMessage.toInt());
        }
    }

    message.id_sender = id;
    message.player_status = player_status;

    //printMessageDetails(message);
}

void Communication::printMessageDetails(const Msg& message) {
    Serial.print("Msg - ID Send: ");
    Serial.print(message.id_sender);
    Serial.print(", Player Status: ");
    Serial.println(playerStatusToString(message.player_status));
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

    LoRa.beginPacket();
    LoRa.print(String(id));
    LoRa.print(" ");
    LoRa.print(String(sensitivity));
    LoRa.print(" ");
    LoRa.print(String(game_state));
    LoRa.print(" ");
    LoRa.print(String(player_status));
    LoRa.print(" ");
    LoRa.print("E");
    LoRa.endPacket();
}

void Communication::resetMsg() {
    message.id_sender = 0;
    message.player_status = PlayerStatus::IDLE;
}

bool Communication::establishedCommunication(int playerId) {
    static unsigned long lastMillis = millis();

    switch (currentState) {
        case CommunicationState::WaitingForEstablishMessage:
            // if (receiveData() == MessageType::ESTABLISH) {
            //     Serial.println("Moving to SendingEstablishMessage state.");
            //     currentState = CommunicationState::SendingEstablishMessage;
            //     lastMillis = millis();
            // }
            break;
        case CommunicationState::SendingEstablishMessage:
            if (millis() - lastMillis > 2000) {
                sendMessage(playerId, 1, GameState::PRE_GAME, PlayerStatus::IDLE);
                Serial.println("Establish message sent by me, the Brain. Waiting for player to send.");
                lastMillis = millis();
            }
            // if (receiveData() == MessageType::ESTABLISH) {
            //     currentState = CommunicationState::Completed;
            // }
            break;
        case CommunicationState::Completed:
            return true;
    }
    return false;
}
