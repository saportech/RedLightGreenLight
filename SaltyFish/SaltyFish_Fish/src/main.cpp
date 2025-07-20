#include <Arduino.h>
#include <vector>
#include "Game.h"
#include "Com.h"
#include "Player.h"
#include "UI.h"
#include <deque>
#include <Preferences.h>
Preferences preferences;

#define NUM_PLAYERS 10
std::vector<int> playerIDs(NUM_PLAYERS);

int brainState = 0;
unsigned long lastOffset = 0;
unsigned long currentOffset = 0;
unsigned long nextChangeMillis = 0;

Com comm;
Player players[NUM_PLAYERS];
Game game;
UI ui;
BUTTON_PRESSED pressedButton;

void brainStateMachine();
void sendMessageToAllPlayers(GameState state);
void handleGameState(GameState newGameState);
void uiUpdate();
void resetValues(int resetType);
unsigned long getRandomTime(unsigned long minTime, unsigned long maxTime);
const char* getStateName(int state);
const char* getGameStateName(GameState gameState);
void saveStateToNVS();
void loadStateFromNVS();
void loopAnalysis();

class SoundQueue {
    private:
        std::deque<SOUND_TYPE> queue;
        unsigned long lastSoundTime = 0;
        const unsigned long soundDuration = 4500;
    
    public:
        void enqueue(SOUND_TYPE soundID) {
            queue.push_back(soundID);
            saveToNVS();
        }

        void clear() {
            queue.clear();
            saveToNVS();  // Reflect the cleared queue in NVS
        }
    
        void enqueuePriority(SOUND_TYPE soundID) {
            queue.push_front(soundID);
            saveToNVS();
        }
    
        void update(UI& ui) {
            unsigned long now = millis();
            if (now - lastSoundTime >= soundDuration) {
                if (!queue.empty()) {
                    SOUND_TYPE nextSound = queue.front();
                    queue.pop_front();
                    ui.playSound(nextSound);
                    lastSoundTime = now;
                    saveToNVS(); // Save updated queue
                }
            }
        }
    
        void saveToNVS() {
            Preferences prefs;
            prefs.begin("sound", false);
            prefs.putUInt("count", queue.size());
            int i = 0;
            for (auto sound : queue) {
                prefs.putUInt(("s" + String(i)).c_str(), (uint16_t)sound);
                i++;
            }
            prefs.putULong("last", lastSoundTime);
            prefs.end();
        }
    
        void loadFromNVS() {
            Preferences prefs;
            prefs.begin("sound", true);
            uint32_t count = prefs.getUInt("count", 0);
            queue.clear();
            for (uint32_t i = 0; i < count; i++) {
                uint16_t val = prefs.getUInt(("s" + String(i)).c_str(), 0);
                queue.push_back((SOUND_TYPE)val);
            }
            lastSoundTime = prefs.getULong("last", 0);
            prefs.end();
        }
    };

SoundQueue soundQueue;

void setup() {

    Serial.begin(115200);
    Serial.println("Red Light Green Light Brain Unit");

    game.begin();

    ui.setupPinsAndSensors();

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        playerIDs[i] = i + 1;
    }

    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].begin(playerIDs[i]);
        // Serial.print("Initialized player with ID: ");
        // Serial.println(players[i].getId());
        players[i].setStatus(IDLE);
    }

    comm.begin();

    resetValues(PRE_GAME);

    loadStateFromNVS();
    soundQueue.loadFromNVS();
}

void loop() {

    uiUpdate();

    soundQueue.update(ui);

    brainStateMachine();
    
}

void brainStateMachine() {
    int establishRes = 0;
    static unsigned long previousMillis = 0;
    static unsigned long currentChangeTime = 0;
    static bool firstTimeAfterLoad = true;

    Com::Msg message;

    enum RED_GREEN_STATE_TYPE { 
        CHECK_COMMUNICATION,
        START,
        GREEN_LIGHT, 
        WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT,
        STATE_GAMEOVER
    };

    static unsigned long lastPrintMillis = 0;
    if (millis() - lastPrintMillis >= 5000) {
        Serial.print("previousMillis: ");
        Serial.print(previousMillis);
        Serial.print(" currentMillis: ");
        Serial.print(millis());
        Serial.print(" nextMillis: ");
        Serial.print(nextChangeMillis);
        Serial.print(" lastOffset: ");
        Serial.print(lastOffset);
        Serial.print(" Current Change Time: ");
        Serial.println(currentChangeTime);
        // Serial.print(" Brain State: ");
        // Serial.print(getStateName(brainState));
        // Serial.print(", Game State: ");
        // Serial.print(getGameStateName(game.getState()));
        // Serial.print(" Game Mode: ");
        // Serial.print(game.getGameMode() == INDIVIDUAL_AUTOMATIC ? "INDIVIDUAL_AUTOMATIC" : "INDIVIDUAL_MANUAL");
        // Serial.print(" Sensitivity: ");
        // Serial.println(game.getSensitivity());
        lastPrintMillis = millis();
        //saveStateToNVS();
    }

    comm.receiveData();
    sendMessageToAllPlayers(game.getState());

    ui.updateLEDs(game.getState(),game.getGameMode(), players, NUM_PLAYERS, game.getSensitivity());   

    if (pressedButton == END_GAME_PRESSED) {
        handleGameState(GAME_OVER);
        delay(400);
        brainState = STATE_GAMEOVER;
        saveStateToNVS();
    }

    if (game.getGameMode() == INDIVIDUAL_AUTOMATIC) {
        currentChangeTime = millis() - previousMillis + lastOffset;
        currentOffset = millis() - previousMillis + lastOffset;

        if (currentChangeTime >= nextChangeMillis) {
            if (brainState == GREEN_LIGHT || (brainState == START && game.getState() == GAME_BEGIN)) {
                comm.resetMsg();
                handleGameState(RED);
                brainState = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
                nextChangeMillis = getRandomTime(6000, 15000); // Set a new random time
            } else if (brainState == WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT) {
                handleGameState(GREEN);
                brainState = GREEN_LIGHT;
                nextChangeMillis = 4500; // Set a new random time
            }
            previousMillis = millis();
            lastOffset = 0;
            saveStateToNVS();
        }
    }
    
    switch (brainState) {
        case CHECK_COMMUNICATION:
            message = comm.getMsg();
            establishRes = comm.establishedCommunication(message, playerIDs);
            if (establishRes != 0) {
                players[establishRes - 1].setStatus(ESTABLISHED_COMMUNICATION);
            }
            if (establishRes == comm.brainId) {
                Serial.println("Communication established with all players");
                soundQueue.enqueuePriority(ALL_PLAYERS_READY_SOUND);
                brainState = START;
                saveStateToNVS();
            }
            comm.resetMsg();
            if (pressedButton == START_GAME_PRESSED && game.getState() != GAME_BEGIN) {
                brainState = START;
                saveStateToNVS();
            }
            break;
        case START:
            if (pressedButton == START_GAME_PRESSED && game.getState() != GAME_BEGIN) {
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    players[i].setStatus(PLAYING);
                }
                handleGameState(GAME_BEGIN);
                saveStateToNVS();
            }
            if (game.getState() == GAME_BEGIN) {
                if (pressedButton == RED_PRESSED) {
                    comm.resetMsg();
                    handleGameState(RED);
                    brainState = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
                    saveStateToNVS();
                }
            }
            break;
        case GREEN_LIGHT: // State is GREEN now
            message = comm.getMsg();
            if (pressedButton == RED_PRESSED) {
                comm.resetMsg();
                handleGameState(RED);
                brainState = WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT;
                saveStateToNVS();
            }
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (message.id_sender == players[i].getId() && message.player_status == CROSSED_FINISH_LINE && players[i].getStatus() == PLAYING) {
                    Serial.println("Player " + String(players[i].getId()) + " crossed finish line");
                    if (players[i].getId() == 1) {
                        soundQueue.enqueue(PLAYER_1_FINISH_SOUND);
                    } else if (players[i].getId() == 2) {
                        soundQueue.enqueue(PLAYER_2_FINISH_SOUND);
                    } else if (players[i].getId() == 3) {
                        soundQueue.enqueue(PLAYER_3_FINISH_SOUND);
                    } else if (players[i].getId() == 4) {
                        soundQueue.enqueue(PLAYER_4_FINISH_SOUND);
                    } else if (players[i].getId() == 5) {
                        soundQueue.enqueue(PLAYER_5_FINISH_SOUND);
                    } else if (players[i].getId() == 6) {
                        soundQueue.enqueue(PLAYER_6_FINISH_SOUND);
                    } else if (players[i].getId() == 7) {
                        soundQueue.enqueue(PLAYER_7_FINISH_SOUND);
                    } else if (players[i].getId() == 8) {
                        soundQueue.enqueue(PLAYER_8_FINISH_SOUND);
                    } else if (players[i].getId() == 9) {
                        soundQueue.enqueue(PLAYER_9_FINISH_SOUND);
                    } else if (players[i].getId() == 10) {
                        soundQueue.enqueue(PLAYER_10_FINISH_SOUND);
                    }
                    players[i].setStatus(CROSSED_FINISH_LINE);
                    saveStateToNVS();
                }
            }
            comm.resetMsg();
            break;
        case WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT: // State is RED now
            message = comm.getMsg();
            if (pressedButton == GREEN_PRESSED) {
                handleGameState(GREEN);
                brainState = GREEN_LIGHT;
                saveStateToNVS();
            }
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (message.id_sender == players[i].getId() && message.player_status == MOVED && players[i].getStatus() == PLAYING) {
                    Serial.println("Player " + String(players[i].getId()) + " moved during red light");
                    if (players[i].getId() == 1) {
                        soundQueue.enqueue(PLAYER_1_MOVED_SOUND);
                    } else if (players[i].getId() == 2) {
                        soundQueue.enqueue(PLAYER_2_MOVED_SOUND);
                    } else if (players[i].getId() == 3) {
                        soundQueue.enqueue(PLAYER_3_MOVED_SOUND);
                    } else if (players[i].getId() == 4) {
                        soundQueue.enqueue(PLAYER_4_MOVED_SOUND);
                    } else if (players[i].getId() == 5) {
                        soundQueue.enqueue(PLAYER_5_MOVED_SOUND);
                    } else if (players[i].getId() == 6) {
                        soundQueue.enqueue(PLAYER_6_MOVED_SOUND);
                    } else if (players[i].getId() == 7) {
                        soundQueue.enqueue(PLAYER_7_MOVED_SOUND);
                    } else if (players[i].getId() == 8) {
                        soundQueue.enqueue(PLAYER_8_MOVED_SOUND);
                    } else if (players[i].getId() == 9) {
                        soundQueue.enqueue(PLAYER_9_MOVED_SOUND);
                    } else if (players[i].getId() == 10) {
                        soundQueue.enqueue(PLAYER_10_MOVED_SOUND);
                    }
                    players[i].setStatus(NOT_PLAYING);
                    saveStateToNVS();
                }
            }

            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (message.id_sender == players[i].getId() && message.player_status == CROSSED_FINISH_LINE && players[i].getStatus() == PLAYING) {
                    Serial.println("Player " + String(players[i].getId()) + " crossed finish line");
                    if (players[i].getId() == 1) {
                        soundQueue.enqueue(PLAYER_1_FINISH_SOUND);
                    } else if (players[i].getId() == 2) {
                        soundQueue.enqueue(PLAYER_2_FINISH_SOUND);
                    } else if (players[i].getId() == 3) {
                        soundQueue.enqueue(PLAYER_3_FINISH_SOUND);
                    } else if (players[i].getId() == 4) {
                        soundQueue.enqueue(PLAYER_4_FINISH_SOUND);
                    } else if (players[i].getId() == 5) {
                        soundQueue.enqueue(PLAYER_5_FINISH_SOUND);
                    } else if (players[i].getId() == 6) {
                        soundQueue.enqueue(PLAYER_6_FINISH_SOUND);
                    } else if (players[i].getId() == 7) {
                        soundQueue.enqueue(PLAYER_7_FINISH_SOUND);
                    } else if (players[i].getId() == 8) {
                        soundQueue.enqueue(PLAYER_8_FINISH_SOUND);
                    } else if (players[i].getId() == 9) {
                        soundQueue.enqueue(PLAYER_9_FINISH_SOUND);
                    } else if (players[i].getId() == 10) {
                        soundQueue.enqueue(PLAYER_10_FINISH_SOUND);
                    }
                    players[i].setStatus(CROSSED_FINISH_LINE);
                    saveStateToNVS();
                }
            }

            comm.resetMsg();
            break;
        case STATE_GAMEOVER:
            resetValues(GAME_OVER);
            saveStateToNVS();
            brainState = START;
            break;
    }
}

void sendMessageToAllPlayers(GameState state) {
    static unsigned long previousMillisMessages = millis();
    static int currentPlayerIndex = 0;
    static unsigned long sendPlayerMillis = millis();

    #define SEND_INTERVAL 100
    #define PLAYER_SEND_INTERVAL (SEND_INTERVAL / NUM_PLAYERS)

    if (millis() - sendPlayerMillis > PLAYER_SEND_INTERVAL) {
        comm.sendMessage(comm.brainId, players[currentPlayerIndex].getId(), game.getSensitivity(), state, players[currentPlayerIndex].getStatus());

        currentPlayerIndex++;
        if (currentPlayerIndex >= NUM_PLAYERS) {
            currentPlayerIndex = 0;
            previousMillisMessages = millis();
        }

        sendPlayerMillis = millis();
    }
}

void handleGameState(GameState newGameState) {
    game.setState(newGameState);
    ui.releaseServoFlag();

    switch (game.getState()) {
        case GAME_BEGIN:
            Serial.print("Game begin");
            soundQueue.enqueuePriority(GAME_BEGIN_SOUND);
            ui.setServo(SERVO_MODE::SERVO_GREEN);
            break;
        case RED:
            Serial.print("Red light");
            soundQueue.enqueuePriority(RED_LIGHT_SOUND);
            ui.setServo(SERVO_MODE::SERVO_RED);
            break;
        case GREEN:
            Serial.print("Green light");
            soundQueue.enqueuePriority(GREEN_LIGHT_SOUND);
            ui.setServo(SERVO_MODE::SERVO_GREEN);
            break;
        case GAME_OVER:
            Serial.print("Game over");
            soundQueue.clear();
            soundQueue.enqueuePriority(GAME_OVER_SOUND);
            break;
    }

    Serial.println(" Sensitivity: " + String(game.getSensitivity()));
}

void uiUpdate() {
    static unsigned long lastButtonPressMillis = 0;
    const unsigned long buttonCooldown = 200;

    pressedButton = ui.buttonPressed();

    if (millis() - lastButtonPressMillis >= buttonCooldown) {
        if (pressedButton == AUTO_MODE_PRESSED) {
            if (game.getGameMode() == INDIVIDUAL_AUTOMATIC) {
                game.setGameMode(INDIVIDUAL_MANUAL);
                saveStateToNVS();
            } else {
                game.setGameMode(INDIVIDUAL_AUTOMATIC);
                saveStateToNVS();
            }
            GameMode currentMode = game.getGameMode();
            Serial.print("Game mode: ");
            Serial.println(currentMode == INDIVIDUAL_AUTOMATIC ? "INDIVIDUAL_AUTOMATIC" : "INDIVIDUAL_MANUAL");
            lastButtonPressMillis = millis();
        } else if (pressedButton == SENSITIVITY_CHANGE_PRESSED) {
            game.setSensitivity(game.getSensitivity() + 1);//if larger than 3, set to 1
            saveStateToNVS();
            Serial.print("Sensitivity: ");
            Serial.println(game.getSensitivity());

            lastButtonPressMillis = millis();
        }
    }

    if (game.getState() == GameState::RED) {
        ui.setServo(SERVO_MODE::SERVO_SPECIAL);
    }


}

void resetValues(int resetType) {
    
    game.setGameMode(INDIVIDUAL_MANUAL);
    ui.releaseServoFlag();

    
    if (resetType == PRE_GAME) {
        game.setState(PRE_GAME);

        for (int i = 0; i < NUM_PLAYERS; i++) {
            players[i].setStatus(IDLE);
        }
    }
    else if (resetType == GAME_OVER) {
        game.setState(GAME_OVER);

        for (int i = 0; i < NUM_PLAYERS; i++) {
            players[i].setStatus(NOT_PLAYING);
        }
    }

    comm.resetMsg();

    ui.updateLEDs(game.getState(), game.getGameMode(), players, NUM_PLAYERS, game.getSensitivity());
    
}

unsigned long getRandomTime(unsigned long minTime, unsigned long maxTime) {
    return random(minTime, maxTime);
}

const char* getStateName(int state) {
    switch (state) {
        case 0: return "CHECK_COMMUNICATION";
        case 1: return "START";
        case 2: return "GREEN_LIGHT";
        case 3: return "WAIT_FOR_MOVEMENT_DETECTION_DURING_RED_LIGHT";
        case 4: return "GREEN_LIGHT_DELAY";
        case 5: return "STATE_GAMEOVER";
        default: return "UNKNOWN_STATE";
    }
}

const char* getGameStateName(GameState gameState) {
    switch (gameState) {
        case PRE_GAME: return "PRE_GAME";
        case GAME_BEGIN: return "GAME_BEGIN";
        case RED: return "RED";
        case GREEN: return "GREEN";
        case GAME_OVER: return "GAME_OVER";
        default: return "UNKNOWN_GAME_STATE";
    }
}

void saveStateToNVS() {
    preferences.begin("game", false);
    preferences.putInt("gameState", (int)game.getState());
    preferences.putInt("gameMode", (int)game.getGameMode());
    preferences.putInt("sensitivity", game.getSensitivity());
    preferences.putInt("brainState", brainState);

    preferences.putULong("lastOffset", currentOffset);
    preferences.putULong("nextMillis", nextChangeMillis);

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        preferences.putInt(("p" + String(i)).c_str(), players[i].getStatus());
    }
    preferences.end();

    soundQueue.saveToNVS(); // save sound queue separately
}

void loadStateFromNVS() {
    preferences.begin("game", true);
    GameState restoredState = (GameState)preferences.getInt("gameState", PRE_GAME);
    GameMode restoredMode = (GameMode)preferences.getInt("gameMode", INDIVIDUAL_MANUAL);
    int restoredSensitivity = preferences.getInt("sensitivity", 1);
    brainState = preferences.getInt("brainState", 0);

    lastOffset = preferences.getULong("lastOffset", 0);
    nextChangeMillis = preferences.getULong("nextMillis", 0);

    game.setState(restoredState);
    game.setGameMode(restoredMode);
    game.setSensitivity(restoredSensitivity);

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        PlayerStatus s = (PlayerStatus)preferences.getInt(("p" + String(i)).c_str(), IDLE);
        players[i].setStatus(s);
    }

    preferences.end();

    soundQueue.loadFromNVS(); // load sound queue separately

    Serial.print("Restored game state: ");
    Serial.println(getGameStateName(game.getState()));
    Serial.print("Restored game mode: ");
    Serial.println(game.getGameMode() == INDIVIDUAL_AUTOMATIC ? "INDIVIDUAL_AUTOMATIC" : "INDIVIDUAL_MANUAL");
    Serial.print("Restored brain state: ");
    Serial.println(getStateName(brainState));
    Serial.print("Restored lastOffset: ");
    Serial.println(lastOffset);
    Serial.print("Restored nextMillis: ");
    Serial.println(nextChangeMillis);
}

void loopAnalysis()
{
  static unsigned long previousMillis = 0;
  static unsigned long lastMillis = 0;
  static unsigned long minLoopTime = 0xFFFFFFFF;
  static unsigned long maxLoopTime = 0;
  static unsigned long loopCounter = 0;

  #define INTERVAL 1000

  unsigned long currentMillis = millis();
  if ( currentMillis - previousMillis > INTERVAL )
  {
    Serial.print( "Loops: " );
    Serial.print( loopCounter );
    Serial.print( " ( " );
    Serial.print( minLoopTime );
    Serial.print( " / " );
    Serial.print( maxLoopTime );
    Serial.println( " )" );
    previousMillis = currentMillis;
    loopCounter = 0;
    minLoopTime = 0xFFFFFFFF;
    maxLoopTime = 0;
  }
  loopCounter++;
  unsigned long loopTime = currentMillis - lastMillis;
  lastMillis = currentMillis;
  if ( loopTime < minLoopTime )
  {
    minLoopTime = loopTime;
  }
  if ( loopTime > maxLoopTime )
  {
    maxLoopTime = loopTime;
  }

}
