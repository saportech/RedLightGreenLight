#include <Arduino.h>
#include <painlessMesh.h>
#include <FastLED.h>

#define DATA_PIN 27
#define NUM_LEDS 3

#define MESH_PREFIX "RedLightGreenLight"
#define MESH_PASSWORD "myMeshPass"
#define MESH_PORT 7777

painlessMesh mesh;
CRGB leds[NUM_LEDS];

void receivedCallback(uint32_t from, String &msg);

void setup() {

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.show();
    //lower intensity
    FastLED.setBrightness(50);
    //show leds for testing
    for (int i = 0; i < 3; i++) {
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(100);
        leds[i] = CRGB::Black;
        FastLED.show();
    }

    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);

    mesh.onReceive([](uint32_t from, String &msg) {
        receivedCallback(from, msg);
    });

    for (int i = 0; i < 3; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}

void loop() {

    mesh.update();

}

void receivedCallback(uint32_t from, String &msg) {
    //Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}
