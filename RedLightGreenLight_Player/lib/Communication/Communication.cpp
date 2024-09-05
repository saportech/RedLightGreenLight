#include "Communication.h"

// Define mesh network parameters
#define MESH_PREFIX "RedLightGreenLight"
#define MESH_PASSWORD "myMeshPass"
#define MESH_PORT 7777

#define DEBUG

// Instantiate the mesh network
painlessMesh mesh;

bool Communication::messageReceived = false;
String Communication::incomingMessage = "";

Communication::Communication() {

}

void Communication::begin(int id) {

    playerId = id;

    currentEstablishState = CommunicationState::WaitingForEstablishMessage;
    
    resetMsg();
    // Set debug message types
    //mesh.setDebugMsgTypes(ERROR | STARTUP);

    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);

    mesh.onReceive([](uint32_t from, String &msg) {
        Communication::receivedCallback(from, msg);
    });
    // mesh.onNewConnection([](uint32_t nodeId) {
    //     Communication::newConnectionCallback(nodeId);
    // });
    // mesh.onChangedConnections([]() {
    //     Communication::changedConnectionCallback();
    // });
    // mesh.onNodeTimeAdjusted([](int32_t offset) {
    //     Communication::nodeTimeAdjustedCallback(offset);
    // });
}

void Communication::receiveData() {
    mesh.update();
    if (messageReceived) {
        parseMessage(incomingMessage);
        messageReceived = false;
    }
}

void Communication::parseMessage(const String& incoming) {

    int id_sender = 0, id_receiver = 0, sensitivity = 0;
    GameState game_state = GameState::PRE_GAME;
    PlayerStatus player_status = PlayerStatus::IDLE;

    int firstSpace = incoming.indexOf(' ');
    int secondSpace = incoming.indexOf(' ', firstSpace + 1);
    int thirdSpace = incoming.indexOf(' ', secondSpace + 1);
    int fourthSpace = incoming.indexOf(' ', thirdSpace + 1);

    id_sender = incoming.substring(0, firstSpace).toInt();
    id_receiver = incoming.substring(firstSpace + 1, secondSpace).toInt();
    sensitivity = incoming.substring(secondSpace + 1, thirdSpace).toInt();
    int gameStateInt = incoming.substring(thirdSpace + 1, fourthSpace).toInt();
    int playerStatusInt = incoming.substring(fourthSpace + 1).toInt();

    game_state = static_cast<GameState>(gameStateInt);
    player_status = static_cast<PlayerStatus>(playerStatusInt);

    if (id_sender == BrainId && (id_receiver == playerId || id_receiver == BrainId)) {
        message.id_sender = id_sender;
        message.id_receiver = id_receiver;
        message.sensitivity = sensitivity;
        message.game_state = game_state;
        message.player_status = player_status;
        #ifdef DEBUG
            printMessageDetails(message);
        #endif
    }

}

void Communication::printMessageDetails(const Msg& message) {
    Serial.print(" Send: " + String(message.id_sender));
    Serial.print(" Recv: " + String(message.id_receiver));
    Serial.print(" Sens: " + String(message.sensitivity));
    Serial.print(" Game: " + String(message.game_state));
    Serial.println(" Player: " + String(message.player_status));
}

Communication::Msg Communication::getMsg() {
    return message;
}

void Communication::sendMessage(int id_sender, int id_receiver, int sensitivity, GameState game_state, PlayerStatus player_status) {
    String msg = String(id_sender) + " " + String(id_receiver) + " " + String(sensitivity) + " " + String(game_state) + " " + String(player_status);
    mesh.sendBroadcast(msg);
}

void Communication::resetMsg() {
    message.id_sender = 0;
    message.id_receiver = 0;
    message.sensitivity = 0;
    message.game_state = GameState::PRE_GAME;
    message.player_status = PlayerStatus::IDLE;
}

void Communication::receivedCallback(uint32_t from, String &msg) {
    //Serial.printf("Received from %u: %s\n", from, msg.c_str());
    messageReceived = true;
    incomingMessage = msg;
}

void Communication::newConnectionCallback(uint32_t nodeId) {
    ///Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void Communication::changedConnectionCallback() {
    //Serial.printf("Changed connections\n");
}

void Communication::nodeTimeAdjustedCallback(int32_t offset) {
    //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

bool Communication::establishedCommunication(Msg message, int playerId) {
    static unsigned long lastMillis = millis();

    switch (currentEstablishState) {
        case CommunicationState::WaitingForEstablishMessage:
            if (message.id_sender == 9 && message.id_receiver == playerId && message.game_state == GameState::PRE_GAME) {
                currentEstablishState = CommunicationState::SendingEstablishMessage;
                sendMessage(playerId, 9, 7, GameState::PRE_GAME, PlayerStatus::IDLE);
                lastMillis = millis();
            }
            break;
        case CommunicationState::SendingEstablishMessage:
            if (millis() - lastMillis > 3000) {
                sendMessage(playerId, 9, 7, GameState::PRE_GAME, PlayerStatus::IDLE);
                lastMillis = millis();
            }
            if (message.id_sender == 9 && message.id_receiver == playerId && message.game_state == GameState::PRE_GAME) {
                //Serial.println("Got the establishment ACK");
                currentEstablishState = CommunicationState::Completed;
            }
            break;
        case CommunicationState::Completed:
            return true;
    }
    return false;
}

void Communication::resetEstablishedCommunication() {
    currentEstablishState = CommunicationState::WaitingForEstablishMessage;
}