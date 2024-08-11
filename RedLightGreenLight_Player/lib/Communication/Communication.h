#ifndef Communication_h
#define Communication_h

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "Game.h"
#include "Player.h"

#define SCK     18
#define MISO    19
#define MOSI    23
#define SS      5
#define RST     15
#define DI0     4

#define BAND    433E6

enum class MessageType {
    ESTABLISH,
    GAME,
    UNKNOWN
};

class Communication {
public:
    struct Msg {
        int id;
        int sensitivity;
        GameState game_state;
        PlayerStatus player_status;
    };

    Communication();
    MessageType receiveData();
    Msg getMsg();
    void sendMessage(int id, int sensitivity, GameState game_state, PlayerStatus player_status);
    void begin();
    bool establishedCommunication(int playerId);
private:
    Msg message;
    bool messageReceived;
    enum class CommunicationState {
        WaitingForEstablishMessage,
        SendingEstablishMessage,
        Completed
    };
    CommunicationState currentState;
};

#endif
