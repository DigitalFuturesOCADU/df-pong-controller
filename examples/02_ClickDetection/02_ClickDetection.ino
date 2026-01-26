/*
 * =============================================================================
 * DFPongController - Click Detection Example
 * =============================================================================
 * 
 * This example demonstrates how to detect button CLICKS (press and release)
 * rather than just holding buttons. Each click sends a single command to
 * move the paddle, giving you precise tap-based control.
 * 
 * DIFFERENCE FROM SIMPLE DIGITAL:
 * -------------------------------
 * - SimpleDigital: Paddle moves continuously while button is HELD
 * - ClickDetection: Paddle moves once per CLICK (press-release cycle)
 * 
 * This is useful for:
 * - Tap-based control schemes
 * - Counting button presses
 * - Detecting specific button events
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your UP button/switch between pin 2 and GND
 * - Connect your DOWN button/switch between pin 3 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * HOW CLICK DETECTION WORKS:
 * --------------------------
 * By tracking the PREVIOUS state of each button, we can detect the exact
 * moment a button transitions from not-pressed to pressed. This happens
 * only ONCE per press, unlike digitalRead() which returns LOW continuously
 * while the button is held.
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
// STATE TRACKING VARIABLES
// =============================================================================
// To detect clicks, we need to track BOTH the current state AND the previous
// state of each button. A "click" is detected when the state changes from
// HIGH (not pressed) to LOW (pressed).

int upButtonState = HIGH;        // Current state of UP button
int lastUpButtonState = HIGH;    // Previous state of UP button

int downButtonState = HIGH;      // Current state of DOWN button
int lastDownButtonState = HIGH;  // Previous state of DOWN button

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
    
    Serial.println("==================================");
    Serial.println("DFPongController - Click Detection");
    Serial.println("==================================");
    
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
    Serial.println("Click detection active!");
    Serial.println("Tap buttons to move paddle.");
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
    // Read the current button states
    // --------------------------------------------------------------------------
    // digitalRead() returns HIGH or LOW
    // With INPUT_PULLUP: LOW means pressed, HIGH means not pressed
    
    upButtonState = digitalRead(upButtonPin);
    downButtonState = digitalRead(downButtonPin);
    
    // --------------------------------------------------------------------------
    // Detect UP button click
    // --------------------------------------------------------------------------
    // A CLICK is detected when:
    // - Current state is LOW (button IS pressed)
    // - Previous state was HIGH (button WAS NOT pressed)
    // This means the button was JUST pressed this loop iteration.
    
    if (upButtonState == LOW && lastUpButtonState == HIGH) 
    {
        // UP button was just clicked!
        Serial.println("UP clicked!");
        controller.sendControl(UP);
    }
    
    // --------------------------------------------------------------------------
    // Detect DOWN button click
    // --------------------------------------------------------------------------
    
    if (downButtonState == LOW && lastDownButtonState == HIGH) 
    {
        // DOWN button was just clicked!
        Serial.println("DOWN clicked!");
        controller.sendControl(DOWN);
    }
    
    // --------------------------------------------------------------------------
    // Send NEUTRAL when no new clicks detected
    // --------------------------------------------------------------------------
    // Note: We only send NEUTRAL when there are no active button presses.
    // This is optional - you could also send NEUTRAL on button RELEASE instead.
    
    if (upButtonState == HIGH && downButtonState == HIGH) 
    {
        controller.sendControl(NEUTRAL);
    }
    
    // --------------------------------------------------------------------------
    // Save current states for next loop iteration
    // --------------------------------------------------------------------------
    // IMPORTANT: This line is what makes click detection work!
    // By saving the current state, we can compare it to the next reading
    // and detect when a change (click) occurs.
    
    lastUpButtonState = upButtonState;
    lastDownButtonState = downButtonState;
}
