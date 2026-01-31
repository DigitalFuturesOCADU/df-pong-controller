/*
 * =============================================================================
 * DFPongController - Gravity Click Example
 * =============================================================================
 * 
 * This example demonstrates a "Flappy Bird" style control mechanism.
 * Gravity constantly pulls you in one direction, and clicking gives you
 * a burst of movement in the opposite direction.
 * 
 * HOW FLAPPY BIRD PHYSICS WORK:
 * -----------------------------
 * 1. You have a VELOCITY (speed + direction)
 * 2. GRAVITY constantly increases velocity (pulls you down)
 * 3. CLICKING sets velocity to a negative value (pushes you up)
 * 4. Your direction depends on whether velocity is positive or negative
 * 
 * Example timeline:
 *   - Velocity starts at 0 (not moving)
 *   - Gravity adds +1 each tick: 0 → 1 → 2 → 3 (falling faster)
 *   - You CLICK! Velocity becomes -5 (jumping up)
 *   - Gravity keeps adding: -5 → -4 → -3 → -2 → -1 → 0 → 1 → 2 (arc!)
 *   - This creates the smooth arc motion of Flappy Bird
 * 
 * HARDWARE SETUP:
 * ---------------
 * - Connect your button/switch between pin 2 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
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
// FLAPPY PHYSICS SETTINGS - TUNE THESE TO CHANGE THE FEEL!
// =============================================================================

// jumpStrength: How hard you "flap" when clicking (negative = upward)
// More negative = stronger jump, floatier feel
// Less negative = weaker jump, need to click more often
// Try values between -3 and -8
int jumpStrength = -5;

// gravity: How much velocity increases each tick (positive = pulls down)
// Higher = heavier, falls faster, more frantic clicking needed
// Lower = floatier, slower falls, more relaxed gameplay
// Try values between 1 and 3
int gravity = 1;

// maxFallSpeed: Terminal velocity - how fast you can fall
// Prevents velocity from going too high when not clicking
// Try values between 3 and 10
int maxFallSpeed = 5;

// physicsInterval: How often physics updates (in milliseconds)
// Lower = smoother but more sensitive
// Higher = choppier but more forgiving
// Try values between 30 and 100
unsigned long physicsInterval = 50;

// =============================================================================
// STATE VARIABLES
// =============================================================================

int velocity = 0;                     // Current velocity (negative=up, positive=down)
int lastButtonState = HIGH;           // For click detection
unsigned long lastPhysicsTime = 0;    // For physics timing

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
    
    Serial.println("================================");
    Serial.println("DFPongController - Gravity Click");
    Serial.println("================================");
    
    pinMode(buttonPin, INPUT_PULLUP);
    
    controller.setControllerNumber(controllerNumber);
    controller.setStatusLED(LED_BUILTIN);
    
    if (controller.begin()) 
    {
        Serial.println("Bluetooth initialized!");
        Serial.println("Click to flap! Gravity pulls you down.");
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
    
    lastPhysicsTime = millis();
}

// =============================================================================
// LOOP
// =============================================================================

void loop() 
{
    controller.update();
    
    // --------------------------------------------------------------------------
    // STEP 1: Detect clicks (the "flap")
    // --------------------------------------------------------------------------
    // When button is pressed, SET velocity to jumpStrength.
    // Note: We SET it, not add to it. This is how Flappy Bird works -
    // each flap gives the same boost regardless of current velocity.
    
    int buttonState = digitalRead(buttonPin);
    
    if (buttonState == LOW && lastButtonState == HIGH)
    {
        // FLAP! Set velocity to jump upward
        velocity = jumpStrength;
        
        Serial.print("FLAP! Velocity: ");
        Serial.println(velocity);
    }
    
    lastButtonState = buttonState;
    
    // --------------------------------------------------------------------------
    // STEP 2: Apply physics on a timer
    // --------------------------------------------------------------------------
    // Gravity ALWAYS adds to velocity, making it more positive over time.
    // This means:
    //   - If velocity is negative (rising), gravity slows you down
    //   - If velocity is positive (falling), gravity speeds you up
    //   - This creates the smooth arc motion!
    
    unsigned long now = millis();
    
    if (now - lastPhysicsTime >= physicsInterval)
    {
        lastPhysicsTime = now;
        
        // Apply gravity (always pulls toward positive/falling)
        velocity = velocity + gravity;
        
        // Cap the fall speed (terminal velocity)
        // Without this, velocity would keep increasing forever
        if (velocity > maxFallSpeed)
        {
            velocity = maxFallSpeed;
        }
    }
    
    // --------------------------------------------------------------------------
    // STEP 3: Send direction based on velocity
    // --------------------------------------------------------------------------
    // Negative velocity = moving upward = send DOWN (toward bottom of screen)
    // Positive velocity = moving downward = send UP (toward top of screen)
    // 
    // (The directions may seem backwards, but this matches the game's
    // coordinate system where we confirmed the behavior works correctly)
    
    if (velocity < 0)
    {
        // Rising (negative velocity) - move toward bottom
        controller.sendControl(DOWN);
    }
    else
    {
        // Falling (positive velocity) - move toward top  
        controller.sendControl(UP);
    }
}
