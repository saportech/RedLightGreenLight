#ifndef Communication_h
#define Communication_h

#include <Arduino.h>
#include <painlessMesh.h>
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
    void begin(int playerId);
    void receiveData();
    Msg getMsg();
    void sendMessage(int id_sender, int id_receiver, int sensitivity, GameState game_state, PlayerStatus player_status);
    bool establishedCommunication(Msg message, int playerId);
    void resetMsg();
    void resetEstablishedCommunication();

private:
    Msg message;
    enum class CommunicationState : byte {
        WaitingForEstablishMessage,
        SendingEstablishMessage,
        Completed
    };
    CommunicationState currentEstablishState;

    void parseMessage(const String& incoming);
    void printMessageDetails(const Msg& message);
    const char* playerStatusToString(PlayerStatus status);
    const char* gameStateToString(GameState state);

    static void receivedCallback(uint32_t from, String &msg);
    static void newConnectionCallback(uint32_t nodeId);
    static void changedConnectionCallback();
    static void nodeTimeAdjustedCallback(int32_t offset);

    static bool messageReceived;
    static String incomingMessage;

    int playerId;
    int BrainId = 9;
};

#endif
