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

#define BRAIN_ID 9

enum class MessageType : byte {
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
    void receiveData(int playerId);
    Msg getMsg();
    void sendMessage(int id, PlayerStatus player_status);
    void begin();
    bool establishedCommunication(int playerId);
    void printMessageDetails(const Msg& message);
    const char* gameStateToString(GameState state);
    const char* playerStatusToString(PlayerStatus status);
    

private:
    Msg message;
    bool messageReceived;
    enum class CommunicationState : byte {
        WaitingForEstablishMessage,
        SendingEstablishMessage,
        Completed
    };
    CommunicationState currentState;

    void parseMessage(const String& incoming, int playerId);

};

#endif
