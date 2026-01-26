/*
 * =============================================================================
 * DFPongController - Simple Digital Example
 * =============================================================================
 * 
 * This example demonstrates how to create a basic Pong controller using
 * two physical buttons (or any digital switches) to control paddle movement.
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your UP button/switch between pin 2 and GND
 * - Connect your DOWN button/switch between pin 3 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * HOW IT WORKS:
 * -------------
 * 1. The library creates a Bluetooth Low Energy (BLE) connection
 * 2. Your Arduino advertises itself with your unique controller number
 * 3. The DF Pong game in your browser connects to your controller
 * 4. Button presses are sent wirelessly to move your paddle
 * 
 * LED STATUS PATTERNS:
 * --------------------
 * - Slow blink (500ms): Disconnected, waiting for game to connect
 * - Fast blink (100ms): Connected, completing handshake
 * - Solid ON: Ready to play!
 * 
 * DF PONG: https://github.com/DigitalFuturesOCADU/df-pong
 * 
 * =============================================================================
 */

// Include the DFPongController library
#include <DFPongController.h>

// =============================================================================
// PIN CONFIGURATION
// =============================================================================
// Define which pins your buttons are connected to.
// Change these if you've wired your buttons to different pins.

int upButtonPin = 2;     // Pin for the UP direction button
int downButtonPin = 3;   // Pin for the DOWN direction button

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================
// Set your unique controller number (1-242)
// Each player needs a different number!

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// CONTROLLER SETUP
// =============================================================================
// Create the controller object - this handles all the Bluetooth communication

DFPongController controller;

// =============================================================================
// SETUP - Runs once when Arduino powers on or resets
// =============================================================================

void setup() 
{
    // --------------------------------------------------------------------------
    // Initialize Serial Monitor for debugging (optional but helpful)
    // --------------------------------------------------------------------------
    Serial.begin(9600);
    
    // Wait a moment for Serial to initialize (helpful for seeing startup messages)
    delay(1000);
    
    Serial.println("=================================");
    Serial.println("DFPongController - Simple Digital");
    Serial.println("=================================");
    
    // --------------------------------------------------------------------------
    // Configure button pins
    // --------------------------------------------------------------------------
    // INPUT_PULLUP means:
    // - The pin is set as an input (reading values)
    // - An internal pull-up resistor is enabled
    // - The pin reads HIGH when the button is NOT pressed
    // - The pin reads LOW when the button IS pressed (connected to GND)
    
    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    
    Serial.println("Button pins configured");
    Serial.print("  UP button: pin ");
    Serial.println(upButtonPin);
    Serial.print("  DOWN button: pin ");
    Serial.println(downButtonPin);
    
    // --------------------------------------------------------------------------
    // Configure the controller
    // --------------------------------------------------------------------------
    
    // Set the controller number from the variable defined above
    controller.setControllerNumber(controllerNumber);
    
    Serial.print("Controller number: ");
    Serial.println(controller.getControllerNumber());
    
    // Set which pin has an LED for status indication
    // LED_BUILTIN is the built-in LED on most Arduino boards
    controller.setStatusLED(LED_BUILTIN);
    
    // --------------------------------------------------------------------------
    // Optional: Enable debug messages (uncomment to see detailed BLE info)
    // --------------------------------------------------------------------------
    // This prints detailed information about BLE connections to Serial Monitor.
    // Useful for troubleshooting, but can make Serial output very busy.
    
    // controller.setDebug(true);
    
    // --------------------------------------------------------------------------
    // Initialize Bluetooth and start advertising
    // --------------------------------------------------------------------------
    // The begin() method returns true if successful, false if there was an error.
    
    Serial.println("Initializing Bluetooth...");
    
    if (controller.begin()) 
    {
        Serial.println("Bluetooth initialized successfully!");
        Serial.println("Waiting for game to connect...");
        Serial.println("(LED will blink slowly until connected)");
    } 
    else 
    {
        // If begin() fails, print an error message
        Serial.println("ERROR: Failed to initialize Bluetooth!");
        Serial.println("Check that your board supports BLE.");
        Serial.println("Supported boards: UNO R4 WiFi, Nano 33 IoT/BLE, ESP32");
        
        // Blink the LED rapidly to indicate an error
        while (true) 
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    
    Serial.println("---------------------------------");
}

// =============================================================================
// LOOP - Runs continuously after setup()
// =============================================================================

void loop() 
{
    // --------------------------------------------------------------------------
    // Update the controller - REQUIRED every loop!
    // --------------------------------------------------------------------------
    // This maintains the Bluetooth connection and updates the status LED.
    // Without this, the controller won't work properly.
    
    controller.update();
    
    // --------------------------------------------------------------------------
    // Read the button states
    // --------------------------------------------------------------------------
    // digitalRead() returns HIGH or LOW
    // With INPUT_PULLUP: LOW means pressed, HIGH means not pressed
    
    int upButtonState = digitalRead(upButtonPin);
    int downButtonState = digitalRead(downButtonPin);
    
    // --------------------------------------------------------------------------
    // Send the appropriate control based on button states
    // --------------------------------------------------------------------------
    // The library provides three constants:
    //   UP      = 1  (paddle moves up)
    //   DOWN    = 2  (paddle moves down)
    //   NEUTRAL = 0  (paddle doesn't move)
    
    if (upButtonState == LOW)           // UP button is pressed
    {
        controller.sendControl(UP);
    } 
    else if (downButtonState == LOW)    // DOWN button is pressed
    {
        controller.sendControl(DOWN);
    } 
    else                                // No button pressed
    {
        controller.sendControl(NEUTRAL);
    }
}
