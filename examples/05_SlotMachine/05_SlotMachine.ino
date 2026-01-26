/*
 * =============================================================================
 * DFPongController - Slot Machine Example
 * =============================================================================
 * 
 * This example creates an unreliable controller where each button has a
 * percentage chance of actually working! Sometimes you press UP and nothing
 * happens. Sometimes it works. It's chaotic and fun!
 * 
 * HOW IT WORKS:
 * -------------
 * - Each button press generates a random number from 1 to 100
 * - If the number is below your success threshold, the button works
 * - If the number is above, nothing happens (sends NEUTRAL instead)
 * 
 * EXAMPLE:
 * - If upSuccessChance = 70, UP works 70% of the time
 * - If downSuccessChance = 50, DOWN works only half the time!
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your UP button/switch between pin 2 and GND
 * - Connect your DOWN button/switch between pin 3 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * TUNING THE CHAOS:
 * -----------------
 * - upSuccessChance: Percentage (1-100) that UP button works
 * - downSuccessChance: Percentage (1-100) that DOWN button works
 * - Set both to 100 for normal operation
 * - Set both to 50 for maximum chaos!
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

int upButtonPin = 2;     // Pin for the UP direction button
int downButtonPin = 3;   // Pin for the DOWN direction button

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================
// Set your unique controller number (1-242)
// Each player needs a different number!

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// CHANCE SETTINGS
// =============================================================================
// Set the percentage chance (1-100) that each button actually works
// 100 = always works, 50 = works half the time, 1 = almost never works

int upSuccessChance = 70;    // UP button works 70% of the time
int downSuccessChance = 70;  // DOWN button works 70% of the time

// =============================================================================
// STATE TRACKING VARIABLES
// =============================================================================

int upButtonState = HIGH;
int downButtonState = HIGH;

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
    
    Serial.println("===============================");
    Serial.println("DFPongController - Slot Machine");
    Serial.println("===============================");
    
    // Configure button pins
    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    
    Serial.print("UP button pin: ");
    Serial.println(upButtonPin);
    Serial.print("DOWN button pin: ");
    Serial.println(downButtonPin);
    
    // Seed the random number generator
    // Using an unconnected analog pin gives us a random starting point
    randomSeed(analogRead(A0));
    
    Serial.println("Random number generator seeded!");
    Serial.print("UP success chance: ");
    Serial.print(upSuccessChance);
    Serial.println("%");
    Serial.print("DOWN success chance: ");
    Serial.print(downSuccessChance);
    Serial.println("%");
    
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
    Serial.println("Will your buttons work?");
    Serial.println("There's only one way to find out!");
    Serial.println("---------------------------------");
}

// =============================================================================
// LOOP - Runs continuously after setup()
// =============================================================================

void loop() 
{
    // Update the controller - REQUIRED every loop!
    controller.update();
    
    // --------------------------------------------------------------------------
    // Read button states
    // --------------------------------------------------------------------------
    
    upButtonState = digitalRead(upButtonPin);
    downButtonState = digitalRead(downButtonPin);
    
    // --------------------------------------------------------------------------
    // Check UP button with random chance
    // --------------------------------------------------------------------------
    
    if (upButtonState == LOW)
    {
        // Generate a random number from 1 to 100
        int roll = random(1, 101);
        
        // Check if we "succeed"
        if (roll <= upSuccessChance)
        {
            // Success! Button works this time
            controller.sendControl(UP);
        }
        else
        {
            // Failed! Button does nothing
            controller.sendControl(NEUTRAL);
        }
    }
    // --------------------------------------------------------------------------
    // Check DOWN button with random chance
    // --------------------------------------------------------------------------
    else if (downButtonState == LOW)
    {
        // Generate a random number from 1 to 100
        int roll = random(1, 101);
        
        // Check if we "succeed"
        if (roll <= downSuccessChance)
        {
            // Success! Button works this time
            controller.sendControl(DOWN);
        }
        else
        {
            // Failed! Button does nothing
            controller.sendControl(NEUTRAL);
        }
    }
    // --------------------------------------------------------------------------
    // No button pressed
    // --------------------------------------------------------------------------
    else
    {
        controller.sendControl(NEUTRAL);
    }
}
