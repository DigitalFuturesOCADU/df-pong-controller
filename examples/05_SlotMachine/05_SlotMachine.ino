/*
 * =============================================================================
 * DFPongController - Slot Machine Example
 * =============================================================================
 * 
 * This example creates a chaotic controller where pressing the button
 * triggers a random move! You don't know which direction or how long.
 * 
 * HOW IT WORKS:
 * -------------
 * 1. Press the button to "pull the lever"
 * 2. Controller picks a random direction (UP or DOWN)
 * 3. Controller picks a random duration
 * 4. It moves that direction for that duration
 * 5. You can't trigger another move until it finishes!
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your button between pin 2 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 * 
 * TUNING THE CHAOS:
 * -----------------
 * - minDuration: Shortest possible move (milliseconds)
 * - maxDuration: Longest possible move (milliseconds)
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

#include <DFPongController.h>

// =============================================================================
// PIN CONFIGURATION
// =============================================================================

int buttonPin = 2;

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// CHAOS SETTINGS - TUNE THESE!
// =============================================================================

// How long each random move lasts (in milliseconds)
unsigned long minDuration = 200;   // Minimum move time
unsigned long maxDuration = 300;   // Maximum move time

// =============================================================================
// STATE VARIABLES
// =============================================================================

bool isMoving = false;               // Are we currently in a random move?
int currentDirection = NEUTRAL;      // Current direction being sent
unsigned long moveEndTime = 0;       // When the current move ends
int lastButtonState = HIGH;          // For click detection

// =============================================================================
// CONTROLLER SETUP
// =============================================================================

DFPongController controller;

// =============================================================================
// SETUP
// =============================================================================

void setup() 
{
    Serial.begin(9600);
    delay(1000);
    
    Serial.println("===============================");
    Serial.println("DFPongController - Slot Machine");
    Serial.println("===============================");
    
    pinMode(buttonPin, INPUT_PULLUP);
    
    // Seed random number generator
    randomSeed(analogRead(A0));
    
    // Configure the controller
    controller.setControllerNumber(controllerNumber);
    controller.setStatusLED(LED_BUILTIN);
    
    Serial.print("Controller number: ");
    Serial.println(controller.getControllerNumber());
    
    if (controller.begin()) 
    {
        Serial.println("Bluetooth initialized!");
        Serial.println("Press the button to pull the lever!");
    } 
    else 
    {
        Serial.println("ERROR: Bluetooth failed!");
        while (true) 
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
}

// =============================================================================
// LOOP
// =============================================================================

void loop() 
{
    controller.update();
    
    int buttonState = digitalRead(buttonPin);
    
    // --------------------------------------------------------------------------
    // Check if current move has finished
    // --------------------------------------------------------------------------
    
    if (isMoving && millis() >= moveEndTime)
    {
        isMoving = false;
        currentDirection = NEUTRAL;
        Serial.println("Move finished! Ready for next pull.");
    }
    
    // --------------------------------------------------------------------------
    // Check for button press (only if not currently moving)
    // --------------------------------------------------------------------------
    
    if (!isMoving && buttonState == LOW && lastButtonState == HIGH)
    {
        // Button pressed - trigger random move!
        isMoving = true;
        
        // Pick random direction
        if (random(0, 2) == 0)
        {
            currentDirection = UP;
            Serial.print("SPIN! Direction: UP, ");
        }
        else
        {
            currentDirection = DOWN;
            Serial.print("SPIN! Direction: DOWN, ");
        }
        
        // Pick random duration
        unsigned long duration = random(minDuration, maxDuration + 1);
        moveEndTime = millis() + duration;
        
        Serial.print("Duration: ");
        Serial.print(duration);
        Serial.println("ms");
    }
    
    lastButtonState = buttonState;
    
    // --------------------------------------------------------------------------
    // Send the current direction
    // --------------------------------------------------------------------------
    
    controller.sendControl(currentDirection);
}
