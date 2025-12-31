/*
 * StartTemplate.ino
 * 
 * A template for creating your own DF Pong controller.
 * This example provides the basic structure - add your own input logic!
 * 
 * How it works:
 * 1. Read your sensor/input in the handleInput() function
 * 2. Set currentDirection to UP, DOWN, or NEUTRAL
 * 3. The library handles all BLE communication automatically
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

// Current direction to send (UP, DOWN, or NEUTRAL)
int currentDirection = NEUTRAL;

void setup() {
    Serial.begin(9600);
    delay(1000);  // Give Serial time to connect
    
    Serial.println("=== DF Pong Controller ===");
    
    // ============================================
    // IMPORTANT: Set YOUR controller number!
    // Each player needs a UNIQUE number (1-242)
    // ============================================
    controller.setControllerNumber(1);  // <-- CHANGE THIS!
    // ============================================
    
    // Optional: Use built-in LED to show connection status
    // - Slow blink = waiting for connection
    // - Fast blink = connected, handshaking
    // - Solid = ready to play
    controller.setStatusLED(LED_BUILTIN);
    
    // Optional: Enable debug messages to Serial
    // controller.setDebug(true);
    
    // ============================================
    // ADD YOUR SENSOR SETUP HERE
    // ============================================
    // Examples:
    // pinMode(buttonPin, INPUT_PULLUP);  // For buttons
    // pinMode(sensorPin, INPUT);         // For analog sensors
    // Wire.begin();                      // For I2C sensors
    // ============================================
    
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
    // Must be called every loop iteration
    controller.update();
    
    // Read inputs and determine direction
    handleInput();
    
    // Send the current direction to the game
    controller.sendControl(currentDirection);
}

// ============================================
// handleInput() - Add your sensor logic here
// ============================================
// This function should read your sensor/input and set
// currentDirection to one of:
//   UP      - paddle moves up
//   DOWN    - paddle moves down  
//   NEUTRAL - paddle stays still
// ============================================
void handleInput() {
    // -----------------------------------------
    // EXAMPLE 1: Two buttons
    // -----------------------------------------
    // bool upPressed = !digitalRead(upButtonPin);    // LOW when pressed
    // bool downPressed = !digitalRead(downButtonPin);
    // 
    // if (upPressed && !downPressed) {
    //     currentDirection = UP;
    // } else if (downPressed && !upPressed) {
    //     currentDirection = DOWN;
    // } else {
    //     currentDirection = NEUTRAL;
    // }
    
    // -----------------------------------------
    // EXAMPLE 2: Analog sensor (potentiometer, joystick)
    // -----------------------------------------
    // int sensorValue = analogRead(sensorPin);  // 0-1023
    // int centerValue = 512;
    // int deadzone = 100;  // Ignore small movements
    // 
    // if (sensorValue > centerValue + deadzone) {
    //     currentDirection = UP;
    // } else if (sensorValue < centerValue - deadzone) {
    //     currentDirection = DOWN;
    // } else {
    //     currentDirection = NEUTRAL;
    // }
    
    // -----------------------------------------
    // EXAMPLE 3: Accelerometer (tilt control)
    // -----------------------------------------
    // float tilt = readAccelerometerY();  // Your sensor reading
    // float threshold = 0.2;  // Tilt sensitivity
    // 
    // if (tilt > threshold) {
    //     currentDirection = UP;
    // } else if (tilt < -threshold) {
    //     currentDirection = DOWN;
    // } else {
    //     currentDirection = NEUTRAL;
    // }
    
    // -----------------------------------------
    // YOUR CODE HERE
    // -----------------------------------------
    // Replace this with your actual input logic
    currentDirection = NEUTRAL;
}
