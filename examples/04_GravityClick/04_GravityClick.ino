/*
 * =============================================================================
 * DFPongController - Gravity Click Example
 * =============================================================================
 * 
 * This example demonstrates a "flappy bird" style control where gravity
 * constantly pulls your paddle DOWN, but clicking gives you lift to go UP.
 * 
 * HOW IT WORKS:
 * -------------
 * - You have a "lift" value that starts at zero
 * - Every click ADDS to your lift (+10)
 * - Gravity constantly SUBTRACTS from your lift over time (-1)
 * - If lift is above zero → paddle moves UP
 * - If lift is zero or below → paddle moves DOWN
 * 
 * The result: You must keep clicking to stay up! Stop clicking and you sink.
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your button/switch between pin 2 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * TUNING THE FEEL:
 * ----------------
 * - liftPerClick: How much each click adds (higher = easier)
 * - gravityRate: How often gravity is applied in milliseconds (lower = harder)
 * - gravityAmount: How much gravity subtracts each time (higher = harder)
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

int buttonPin = 2;   // Pin for the click button

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================
// Set your unique controller number (1-242)
// Each player needs a different number!

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// GRAVITY SETTINGS
// =============================================================================
// Adjust these to change how the controller feels

int liftPerClick = 10;         // How much lift each click gives you
int gravityAmount = 1;         // How much gravity pulls you down each tick
unsigned long gravityRate = 50; // How often gravity is applied (milliseconds)

// =============================================================================
// STATE TRACKING VARIABLES
// =============================================================================

int buttonState = HIGH;        // Current state of the button
int lastButtonState = HIGH;    // Previous state (for click detection)

int lift = 0;                  // Current lift value (can go negative)

unsigned long lastGravityTime = 0;  // When gravity was last applied

// =============================================================================
// CONTROLLER SETUP
// =============================================================================

DFPongController controller;

// =============================================================================
// SETUP - Runs once when Arduino powers on or resets
// =============================================================================

void setup() 
{
    // Initialize Serial Monitor for debugging
    Serial.begin(9600);
    delay(1000);
    
    Serial.println("================================");
    Serial.println("DFPongController - Gravity Click");
    Serial.println("================================");
    
    // Configure button pin
    pinMode(buttonPin, INPUT_PULLUP);
    
    Serial.print("Button pin: ");
    Serial.println(buttonPin);
    
    // Configure the controller
    controller.setControllerNumber(controllerNumber);
    
    Serial.print("Controller number: ");
    Serial.println(controller.getControllerNumber());
    
    controller.setStatusLED(LED_BUILTIN);
    
    // Initialize Bluetooth
    Serial.println("Initializing Bluetooth...");
    
    if (controller.begin()) 
    {
        Serial.println("Bluetooth initialized successfully!");
        Serial.println("Waiting for game to connect...");
    } 
    else 
    {
        Serial.println("ERROR: Failed to initialize Bluetooth!");
        
        // Blink rapidly to indicate error
        while (true) 
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    
    Serial.println("---------------------------------");
    Serial.println("Click to go UP!");
    Serial.println("Stop clicking and gravity pulls");
    Serial.println("you DOWN!");
    Serial.println("---------------------------------");
    
    // Initialize gravity timer
    lastGravityTime = millis();
}

// =============================================================================
// LOOP - Runs continuously after setup()
// =============================================================================

void loop() 
{
    // Update the controller - REQUIRED every loop!
    controller.update();
    
    // --------------------------------------------------------------------------
    // Read button and detect clicks
    // --------------------------------------------------------------------------
    
    buttonState = digitalRead(buttonPin);
    
    // Check for a click (button just pressed)
    if (buttonState == LOW && lastButtonState == HIGH) 
    {
        // Add lift when clicked
        lift = lift + liftPerClick;
        
        Serial.print("CLICK! Lift: ");
        Serial.println(lift);
    }
    
    // Save button state for next loop
    lastButtonState = buttonState;
    
    // --------------------------------------------------------------------------
    // Apply gravity over time
    // --------------------------------------------------------------------------
    
    unsigned long currentTime = millis();
    
    if (currentTime - lastGravityTime >= gravityRate) 
    {
        // Subtract gravity from lift
        lift = lift - gravityAmount;
        
        // Update the timer
        lastGravityTime = currentTime;
    }
    
    // --------------------------------------------------------------------------
    // Send control based on lift value
    // --------------------------------------------------------------------------
    // Lift above zero = going UP
    // Lift at or below zero = going DOWN
    // No NEUTRAL state - you're always moving!
    
    if (lift > 0) 
    {
        controller.sendControl(UP);
    } 
    else 
    {
        controller.sendControl(DOWN);
    }
}
