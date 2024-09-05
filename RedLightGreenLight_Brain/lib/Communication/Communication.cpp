#include "Communication.h"

// Define mesh network parameters
#define MESH_PREFIX "RedLightGreenLight"
#define MESH_PASSWORD "myMeshPass"
#define MESH_PORT 7777

// Instantiate the mesh network
painlessMesh mesh;

bool Communication::messageReceived = false;
String Communication::incomingMessage = "";

Communication::Communication() {
    // No additional setup needed
}

void Communication::begin() {
    resetMsg();

    // Set debug message types
    mesh.setDebugMsgTypes(ERROR | STARTUP);

    // Initialize mesh
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    
    // Register callbacks
    mesh.onReceive([](uint32_t from, String &msg) {
        Communication::receivedCallback(from, msg);
    });

    mesh.onNewConnection([](uint32_t nodeId) {
        Communication::newConnectionCallback(nodeId);
    });
    mesh.onChangedConnections([]() {
        Communication::changedConnectionCallback();
    });
    mesh.onNodeTimeAdjusted([](int32_t offset) {
        Communication::nodeTimeAdjustedCallback(offset);
    });  

    establishedCommunicationIsValid = false;
}

void Communication::sendMessage(int id_sender, int id_receiver, int sensitivity, GameState game_state, PlayerStatus player_status) {
    String msg = String(id_sender) + " " + String(id_receiver) + " " + String(sensitivity) + " " + game_state + " " + player_status;
    if (mesh.getNodeList().size() > 0) {
        mesh.sendBroadcast(msg);
    }
    
    //Serial.println("Sent message: " + msg);
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

    message.id_sender = id_sender;
    message.id_receiver = id_receiver;
    message.sensitivity = sensitivity;
    message.game_state = game_state;
    message.player_status = player_status;

    printMessageDetails(message);
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

void Communication::resetMsg() {
    message.id_sender = 0;
    message.id_receiver = 0;
    message.sensitivity = 0;
    message.game_state = GameState::PRE_GAME;
    message.player_status = PlayerStatus::IDLE;
}

void Communication::receivedCallback(uint32_t from, String &msg) {
    //Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
    messageReceived = true;
    incomingMessage = msg;
}

void Communication::newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void Communication::changedConnectionCallback() {
    Serial.printf("Changed connections\n");
}

void Communication::nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

int Communication::establishedCommunication(Msg message, const std::vector<int>& playerIDs) {
    enum ESTABLISH_STATE_TYPE { 
        ESTABLISHING_COMMUNICATION,
        COMPLETED
    };
    
    static int establishCurrentState = ESTABLISHING_COMMUNICATION;
    static bool allAcknowledged = false;
    
    switch (establishCurrentState) {
        case ESTABLISHING_COMMUNICATION:
            for (size_t i = 0; i < playerIDs.size(); ++i) {
                if (message.id_sender == playerIDs[i] && message.game_state == GameState::PRE_GAME && message.player_status == PlayerStatus::IDLE) {
                    sendMessage(9, playerIDs[i], 7, GameState::PRE_GAME, PlayerStatus::IDLE);
                    Serial.println("Player " + String(playerIDs[i]) + " acknowledged");
                    playerAcknowledged[i] = true;
                    return i + 1;
                }
            }
            allAcknowledged = true;
            for (size_t i = 0; i < playerIDs.size(); ++i) {
                if (!playerAcknowledged[i]) {
                    allAcknowledged = false;
                    break;
                }
            }
            if (allAcknowledged) {
                establishCurrentState = COMPLETED;
                for (size_t i = 0; i < playerIDs.size(); ++i) {
                    playerAcknowledged[i] = false;
                }
                establishedCommunicationIsValid = true;
            }
            break;
        case COMPLETED:
            if (establishedCommunicationIsValid) {
                return 9;
            }
            else {
                establishCurrentState = ESTABLISHING_COMMUNICATION;
            }
            break;
    }

    return 0;
}

void Communication::resetEstablishedCommunication() {
    establishedCommunicationIsValid = false;
}
