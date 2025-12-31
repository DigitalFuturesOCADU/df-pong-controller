/*
 * SimpleDigital.ino
 * 
 * Simple 2-button DF Pong controller example.
 * Press UP button to move paddle up, DOWN button to move paddle down.
 * 
 * Hardware:
 * - Button on pin 2 (UP) - connected to GND
 * - Button on pin 3 (DOWN) - connected to GND
 * - Built-in LED shows connection status
 * 
 * Supported Boards:
 * - Arduino UNO R4 WiFi
 * - Arduino Nano 33 IoT
 * - Arduino Nano 33 BLE / BLE Sense
 * - ESP32 (requires NimBLE-Arduino library)
 * 
 * Test: https://digitalfuturesocadu.github.io/df-pong/game/test/
 * Game: https://digitalfuturesocadu.github.io/df-pong/
 */

#include <DFPongController.h>

// Create the controller object
DFPongController controller;

// Button pins
const int BUTTON_UP = 2;
const int BUTTON_DOWN = 3;

void setup() {
    Serial.begin(9600);
    delay(1000);  // Give Serial time to connect
    
    Serial.println("=== DF Pong Controller ===");
    
    // Configure button pins with internal pullup resistors
    // Buttons read LOW when pressed, HIGH when released
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    
    // ============================================
    // IMPORTANT: Set YOUR controller number!
    // Each player needs a UNIQUE number (1-242)
    // ============================================
    controller.setControllerNumber(1);  // <-- CHANGE THIS!
    // ============================================
    
    // Optional: Use built-in LED to show connection status
    // - Slow blink = waiting for connection
    // - Fast blink = connected, handshaking
    // - Solid = ready to play!
    controller.setStatusLED(LED_BUILTIN);
    
    // Optional: Enable debug messages
    // controller.setDebug(true);
    
    // Start the BLE controller
    if (!controller.begin()) {
        Serial.println("Failed to start BLE!");
        // Blink LED rapidly to indicate error
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
}

void loop() {
    // REQUIRED: Update BLE connection and internal state
    controller.update();
    
    // Read buttons (LOW = pressed because of INPUT_PULLUP)
    bool upPressed = !digitalRead(BUTTON_UP);
    bool downPressed = !digitalRead(BUTTON_DOWN);
    
    // Send the appropriate control
    if (upPressed && !downPressed) {
        controller.sendControl(UP);
    } 
    else if (downPressed && !upPressed) {
        controller.sendControl(DOWN);
    } 
    else {
        controller.sendControl(NEUTRAL);
    }
}
