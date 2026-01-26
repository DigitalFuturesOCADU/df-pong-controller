/*
 * =============================================================================
 * DFPongController - Start Template
 * =============================================================================
 * 
 * This is a template for creating your own custom Pong controller.
 * It includes all the necessary setup and library methods, but leaves
 * the input method up to you!
 * 
 * USE THIS TEMPLATE WHEN:
 * -----------------------
 * - You want to use custom switches, sensors, or other inputs
 * - You want to experiment with different control methods
 * - You're building an alternative controller with unique materials
 * 
 * WHAT YOU NEED TO ADD:
 * ---------------------
 * 1. Your input hardware setup (buttons, sensors, conductive materials, etc.)
 * 2. Logic to read your inputs
 * 3. Calls to controller.sendControl() based on your input readings
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
// YOUR INPUT CONFIGURATION
// =============================================================================
// Define your input pins and variables here.
// Examples:
//   int upButtonPin = 2;
//   int downButtonPin = 3;
//   int sensorPin = A0;

// TODO: Add your input pin definitions here



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
    // Initialize Serial Monitor for debugging
    // --------------------------------------------------------------------------
    Serial.begin(9600);
    delay(1000);  // Wait for Serial to initialize
    
    Serial.println("=================================");
    Serial.println("DFPongController - Custom Controller");
    Serial.println("=================================");
    
    // --------------------------------------------------------------------------
    // YOUR INPUT SETUP
    // --------------------------------------------------------------------------
    // Configure your input pins here.
    // Examples:
    //   pinMode(upButtonPin, INPUT_PULLUP);
    //   pinMode(downButtonPin, INPUT_PULLUP);
    //   pinMode(sensorPin, INPUT);
    
    // TODO: Add your input pin setup here
    
    
    
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
    // controller.setDebug(true);
    
    // --------------------------------------------------------------------------
    // Initialize Bluetooth and start advertising
    // --------------------------------------------------------------------------
    Serial.println("Initializing Bluetooth...");
    
    if (controller.begin()) 
    {
        Serial.println("Bluetooth initialized successfully!");
        Serial.println("Waiting for game to connect...");
    } 
    else 
    {
        Serial.println("ERROR: Failed to initialize Bluetooth!");
        Serial.println("Check that your board supports BLE.");
        
        // Halt with rapid LED blink to indicate error
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
    // YOUR INPUT READING
    // --------------------------------------------------------------------------
    // Read your inputs here.
    // Examples:
    //   int upState = digitalRead(upButtonPin);
    //   int downState = digitalRead(downButtonPin);
    //   int sensorValue = analogRead(sensorPin);
    
    // TODO: Add your input reading code here
    
    
    
    // --------------------------------------------------------------------------
    // SEND CONTROLS
    // --------------------------------------------------------------------------
    // Based on your input readings, send the appropriate control.
    // 
    // Available commands:
    //   controller.sendControl(UP);       // Move paddle up
    //   controller.sendControl(DOWN);     // Move paddle down
    //   controller.sendControl(NEUTRAL);  // Stop paddle movement
    //
    // Example with buttons:
    //   if (upState == LOW) {
    //       controller.sendControl(UP);
    //   } else if (downState == LOW) {
    //       controller.sendControl(DOWN);
    //   } else {
    //       controller.sendControl(NEUTRAL);
    //   }
    //
    // Example with analog sensor:
    //   if (sensorValue > 600) {
    //       controller.sendControl(UP);
    //   } else if (sensorValue < 400) {
    //       controller.sendControl(DOWN);
    //   } else {
    //       controller.sendControl(NEUTRAL);
    //   }
    
    // TODO: Add your control logic here
    
    // Uncomment and modify these lines based on your input method:
    // controller.sendControl(UP);
    // controller.sendControl(DOWN);
    // controller.sendControl(NEUTRAL);
}

// =============================================================================
// ADDITIONAL HELPER FUNCTIONS (Optional)
// =============================================================================
// You can add your own functions here for more complex input processing.
// Examples:
//   - Debouncing functions for noisy switches
//   - Mapping functions for analog sensors
//   - State tracking for gesture detection

