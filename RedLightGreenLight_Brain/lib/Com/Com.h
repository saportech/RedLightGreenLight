#ifndef Com_h
#define Com_h

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <vector>
#include "Game.h"
#include "Player.h"

// const char* predefinedMacs[] = {
//     "88:13:bf:60:bd:a0",//Speaker
//     "ac:15:18:4b:46:02",//written 2 on it
//     "ac:15:18:4b:45:e6",//written 3 on it
//     "ac:15:18:4b:45:f2",//written 4 on it
//     "ac:15:18:4b:46:52",
// };

const int predefinedIds[] = {
    1, 2, 3, 4, 5
};

enum class MessageType : int {
    ESTABLISH,
    GAME,
    UNKNOWN
};

class Com {
public:
    struct Msg {
        int id_sender;
        int id_receiver;
        int sensitivity;
        GameState game_state;
        PlayerStatus player_status;
    };

    Com();
    void begin();
    void receiveData();
    Msg getMsg();
    void sendMessage(int id_sender, int id_receiver, int sensitivity, GameState game_state, PlayerStatus player_status);
    int establishedCommunication(Msg message, const std::vector<int>& playerIDs);
    void resetMsg();
    void resetEstablishedCommunication();

private:
    Msg message;
    static void onDataReceive(const uint8_t *mac, const uint8_t *data, int len);
    static void onDataSent(const uint8_t *mac, esp_now_send_status_t status);

    void parseMessage(const uint8_t *data, int len);
    void printMessageDetails(const Msg& message);

    static const char* gameStateToString(GameState state);
    static const char* playerStatusToString(PlayerStatus status);

    void addPlayersAsPeers();

    static bool messageReceived;
    static Msg incomingMessage;
    bool establishedCommunicationIsValid;

    static const size_t MAX_PLAYERS = 5;
    bool playerAcknowledged[MAX_PLAYERS];

    static const char* predefinedMacs[5];

};

#endif
