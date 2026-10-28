/*
 * =============================================================================
 * DFPongController - Chorded Inputs Example
 * =============================================================================
 * 
 * This example demonstrates how to use COMBINATIONS of buttons pressed 
 * together (chords) to create different controls. With 4 buttons, you can
 * create many unique input combinations!
 * 
 * WHAT ARE CHORDED INPUTS?
 * ------------------------
 * Like playing a chord on a piano (multiple keys at once), chorded inputs
 * detect when multiple buttons are pressed simultaneously. This allows you
 * to create more controls than you have buttons!
 * 
 * With 4 buttons you can have:
 * - 4 single-button inputs
 * - 6 two-button combinations
 * - 4 three-button combinations
 * - 1 four-button combination
 * = 15 total unique inputs from just 4 buttons!
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect Button 1 between pin 2 and GND
 * - Connect Button 2 between pin 3 and GND
 * - Connect Button 3 between pin 4 and GND
 * - Connect Button 4 between pin 5 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * CONTROL MAPPING:
 * ----------------
 * This example maps button combinations to Pong controls:
 * - Button 1 alone        → UP
 * - Button 2 alone        → DOWN
 * - Buttons 1+2 together  → NEUTRAL (they cancel out!)
 * - Button 3 alone        → UP (alternative)
 * - Button 4 alone        → DOWN (alternative)
 * - Buttons 3+4 together  → NEUTRAL (they cancel out!)
 * - Buttons 1+3 together  → UP (reinforced!)
 * - Buttons 2+4 together  → DOWN (reinforced!)
 * - All 4 buttons         → NEUTRAL (full stop!)
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

int button1Pin = 2;   // Button 1 - Primary UP
int button2Pin = 3;   // Button 2 - Primary DOWN
int button3Pin = 4;   // Button 3 - Secondary UP
int button4Pin = 5;   // Button 4 - Secondary DOWN

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================
// Set your unique controller number (1-242)
// Each player needs a different number!

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// STATE TRACKING VARIABLES
// =============================================================================
// Current state of each button (HIGH = not pressed, LOW = pressed)

int button1State = HIGH;
int button2State = HIGH;
int button3State = HIGH;
int button4State = HIGH;

// Previous state of each button (for detecting changes)

int lastButton1State = HIGH;
int lastButton2State = HIGH;
int lastButton3State = HIGH;
int lastButton4State = HIGH;

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
    Serial.println("DFPongController - Chorded Inputs");
    Serial.println("=================================");
    
    // --------------------------------------------------------------------------
    // Configure button pins
    // --------------------------------------------------------------------------
    // INPUT_PULLUP means:
    // - The pin is set as an input (reading values)
    // - An internal pull-up resistor is enabled
    // - The pin reads HIGH when the button is NOT pressed
    // - The pin reads LOW when the button IS pressed (connected to GND)
    
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);
    pinMode(button3Pin, INPUT_PULLUP);
    pinMode(button4Pin, INPUT_PULLUP);
    
    Serial.println("Button pins configured:");
    Serial.print("  Button 1 (UP):   pin ");
    Serial.println(button1Pin);
    Serial.print("  Button 2 (DOWN): pin ");
    Serial.println(button2Pin);
    Serial.print("  Button 3 (UP):   pin ");
    Serial.println(button3Pin);
    Serial.print("  Button 4 (DOWN): pin ");
    Serial.println(button4Pin);
    
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
    Serial.println("Chorded inputs active!");
    Serial.println("Try pressing buttons individually");
    Serial.println("or in combinations!");
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
    // Read all button states
    // --------------------------------------------------------------------------
    // digitalRead() returns HIGH or LOW
    // With INPUT_PULLUP: LOW means pressed, HIGH means not pressed
    
    button1State = digitalRead(button1Pin);
    button2State = digitalRead(button2Pin);
    button3State = digitalRead(button3Pin);
    button4State = digitalRead(button4Pin);
    
    // --------------------------------------------------------------------------
    // Determine which control to send based on button combination
    // --------------------------------------------------------------------------
    // The determineDirection() function counts UP buttons vs DOWN buttons:
    // - Buttons 1 and 3 are "UP" buttons
    // - Buttons 2 and 4 are "DOWN" buttons
    // - Returns UP, DOWN, or NEUTRAL based on which has more
    
    int direction = determineDirection();
    controller.sendControl(direction);
    
    // --------------------------------------------------------------------------
    // Save current states for next loop iteration
    // --------------------------------------------------------------------------
    // These aren't used in this simple version, but are here if you want to
    // add change detection later (e.g., print only when buttons change)
    
    lastButton1State = button1State;
    lastButton2State = button2State;
    lastButton3State = button3State;
    lastButton4State = button4State;
}

// =============================================================================
// HELPER FUNCTION - Determine direction from button states
// =============================================================================
// This function counts how many UP buttons vs DOWN buttons are pressed
// and returns UP, DOWN, or NEUTRAL based on which has more.
//
// Returns:
//   UP      - if more UP buttons are pressed than DOWN buttons
//   DOWN    - if more DOWN buttons are pressed than UP buttons  
//   NEUTRAL - if equal (including when no buttons are pressed)

int determineDirection()
{
    // Count how many UP buttons are pressed
    int upCount = 0;
    
    if (button1State == LOW)
    {
        upCount = upCount + 1;
    }
    
    if (button3State == LOW)
    {
        upCount = upCount + 1;
    }
    
    // Count how many DOWN buttons are pressed
    int downCount = 0;
    
    if (button2State == LOW)
    {
        downCount = downCount + 1;
    }
    
    if (button4State == LOW)
    {
        downCount = downCount + 1;
    }
    
    // Compare counts and return the appropriate direction
    if (upCount > downCount)
    {
        return UP;
    }
    else if (downCount > upCount)
    {
        return DOWN;
    }
    else
    {
        return NEUTRAL;
    }
}
