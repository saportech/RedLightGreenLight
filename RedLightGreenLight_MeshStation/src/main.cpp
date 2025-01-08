#include <Arduino.h>
#include <painlessMesh.h>
#include <FastLED.h>

#define DATA_PIN 27
#define NUM_LEDS 4

#define MESH_PREFIX "RedLightGreenLight"
#define MESH_PASSWORD "myMeshPass"
#define MESH_PORT 7777

painlessMesh mesh;
CRGB leds[NUM_LEDS];

void setupLeds();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();

void setup() {

    Serial.begin(115200);
    Serial.println("Mesh station");

    setupLeds();

    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);

    mesh.onReceive([](uint32_t from, String &msg) {
        receivedCallback(from, msg);
    });
    mesh.onNewConnection([](uint32_t nodeId) {
        newConnectionCallback(nodeId);
    });
    mesh.onChangedConnections([]() {
        changedConnectionCallback();
    });


}

void loop() {
    static unsigned long lastMessage = millis();

    mesh.update();

    if (millis() - lastMessage > 50) {
        //Serial.println("Sending message" + String(mesh.getNodeId()));
        lastMessage = millis();
        mesh.sendBroadcast("Hello from node " + String(mesh.getNodeId()));
    }

    

}

void setupLeds() {

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.show();
    FastLED.setBrightness(10);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(100);
        leds[i] = CRGB::Black;
        FastLED.show();
    }

    for (int i = 0; i < 4; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}

void receivedCallback(uint32_t from, String &msg) {
    Serial.println("Received from " + String(from) + " msg=" + msg);
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections\n");
}
