#ifndef Communication_h
#define Communication_h

#include <Arduino.h>
#include <painlessMesh.h>
#include <vector>
#include "Game.h"
#include "Player.h"

enum class MessageType : int {
    ESTABLISH,
    GAME,
    UNKNOWN
};

class Communication {
public:
    struct Msg {
        int id_sender;
        int id_receiver;
        int sensitivity;
        GameState game_state;
        PlayerStatus player_status;
    };

    Communication();
    void begin();
    void receiveData();
    Msg getMsg();
    void sendMessage(int id_sender, int id_receiver, int sensitivity, GameState game_state, PlayerStatus player_status);
    int establishedCommunication(Msg message, const std::vector<int>& playerIDs);

    void resetMsg();
    void resetEstablishedCommunication();

private:
    Msg message;
    enum class CommunicationState {
        WaitingForEstablishMessages,
        Completed
    };

    void parseMessage(const String& incoming);
    void printMessageDetails(const Msg& message);

    static void receivedCallback(uint32_t from, String &msg);
    static void newConnectionCallback(uint32_t nodeId);
    static void changedConnectionCallback();
    static void nodeTimeAdjustedCallback(int32_t offset);

    static bool messageReceived;
    static String incomingMessage;
    bool establishedCommunicationIsValid;

    static const size_t MAX_PLAYERS = 10;
    bool playerAcknowledged[MAX_PLAYERS];
};

#endif
