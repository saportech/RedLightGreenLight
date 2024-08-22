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

void Communication::receiveData(int playerId) {
    int packetSize = LoRa.parsePacket();
    String incoming = "";

    if (packetSize) {
        while (LoRa.available()) {
            incoming += (char)LoRa.read();
        }
        Serial.println("Received: " + incoming);

        if (incoming.endsWith(" E")) {
            incoming.remove(incoming.length() - 2);
        }

        parseMessage(incoming, playerId);
    }
}

void Communication::parseMessage(const String& incoming, int playerId) {
    int sensitivity = 0;
    GameState game_state = GameState::GAME_OVER;
    PlayerStatus player_status = PlayerStatus::NOT_PLAYING;
    int id = 0;

    int index = 0;
    String token = "";
    int part = 0;
    String LoRaMessage = incoming;
    int spaceCount = 0;

    while ((index = LoRaMessage.indexOf(' ')) != -1) {
        spaceCount++;
        token = LoRaMessage.substring(0, index);
        LoRaMessage = LoRaMessage.substring(index + 1);

        switch (part) {
            case 0:
                id = token.toInt();
                break;
            case 1:
                sensitivity = token.toInt();
                break;
            case 2:
                game_state = static_cast<GameState>(token.toInt());
                break;
            case 3:
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
            sensitivity = LoRaMessage.toInt();
        } else if (part == 2) {
            game_state = static_cast<GameState>(LoRaMessage.toInt());
        } else if (part == 3) {
            player_status = static_cast<PlayerStatus>(LoRaMessage.toInt());
        }
    }
    if (spaceCount == 3 && (id == BRAIN_ID || id == playerId)) {
        message.id_sender = BRAIN_ID;
        message.id_receiver = id;
        message.sensitivity = sensitivity;
        message.game_state = game_state;
        message.player_status = player_status;

        messageReceived = true;
        //printMessageDetails(message);
    }

}

void Communication::printMessageDetails(const Msg& message) {
    Serial.print("Msg - ID Send: ");
    Serial.print(message.id_sender);
    Serial.print(", ID Recv: ");
    Serial.print(message.id_receiver);
    Serial.print(", Sens: ");
    Serial.print(message.sensitivity);
    Serial.print(", Game State: ");
    Serial.print(message.game_state);
    Serial.print(", Player Status: ");
    Serial.println(message.player_status);
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

void Communication::sendMessage(int id, PlayerStatus player_status) {
    LoRa.beginPacket();
    LoRa.print(String(id));
    LoRa.print(" ");
    LoRa.print(String(static_cast<int>(player_status)));
    LoRa.print(" ");
    LoRa.print("E");
    LoRa.endPacket();
}

bool Communication::establishedCommunication(int playerId) {
    static unsigned long lastMillis = millis();

    switch (currentState) {
        case CommunicationState::WaitingForEstablishMessage:
            //if (receiveData() == MessageType::ESTABLISH) {
                Serial.println("Moving to SendingEstablishMessage state.");
                currentState = CommunicationState::SendingEstablishMessage;
                lastMillis = millis();
            //}
            break;
        case CommunicationState::SendingEstablishMessage:
            if (millis() - lastMillis > 2000) {
                //sendMessage(playerId, 1, GameState::PRE_GAME, PlayerStatus::IDLE);
                Serial.println("Establish message sent by me, the Brain. Waiting for player to send.");
                lastMillis = millis();
            }
            //if (receiveData() == MessageType::ESTABLISH) {
                currentState = CommunicationState::Completed;
            //}
            break;
        case CommunicationState::Completed:
            return true;
    }
    return false;
}
