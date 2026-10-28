/*
 * =============================================================================
 * DFPongController - Live Settings Example
 * =============================================================================
 *
 * Settings don't have to be fixed in setup() - you can change them WHILE
 * you play! In this example a potentiometer (knob) changes the color of
 * your paddle live in the game. Turn the knob and watch your paddle change
 * through all the colors of the rainbow.
 *
 * Two buttons move the paddle, just like 01_SimpleDigital.
 *
 * NOTE: Custom settings need the 2026 version of the DF Pong game.
 *
 * HARDWARE SETUP:
 * ---------------
 * - Connect your UP button/switch between pin 2 and GND
 * - Connect your DOWN button/switch between pin 3 and GND
 * - Connect a potentiometer (10k is fine):
 *     one outer leg  -> 3.3V
 *     other outer leg -> GND
 *     middle leg      -> A0
 *   (Use 3.3V, not 5V: the Nano 33 IoT and ESP32 pins only handle 3.3V.)
 *
 * HOW LIVE SETTINGS WORK:
 * -----------------------
 * - Call a setting function (like setPlayerColor) any time, even in loop().
 * - The library sends the new settings to the game automatically during
 *   controller.update(), at most 4 times per second. If you change things
 *   faster than that, only the newest values are sent.
 * - Calling a setting with the SAME value again sends nothing, but it's
 *   still good practice to only call it when something really changed
 *   (that's what this example does).
 * - PLAYER settings (name, color, speed, size, sounds) change right away.
 *   GAME requests (like requestPointsToWin) apply to the NEXT match.
 *
 * LED STATUS PATTERNS:
 * --------------------
 * - Slow blink (500ms): Disconnected, waiting for game to connect
 * - Fast blink (100ms): Connected, completing handshake
 * - Solid ON: Ready to play!
 * (Some ESP32 boards have no LED_BUILTIN. Then there is no status LED -
 *  use your LED's pin number instead, e.g. controller.setStatusLED(5);)
 *
 * DF PONG: https://github.com/DigitalFuturesOCADU/df-pong
 *
 * =============================================================================
 */

#include <DFPongController.h>

// =============================================================================
// PIN CONFIGURATION
// =============================================================================

int upButtonPin = 2;     // Pin for the UP direction button
int downButtonPin = 3;   // Pin for the DOWN direction button
int knobPin = A0;        // Middle leg of the potentiometer

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// COLOR SETTINGS
// =============================================================================

// The knob picks one of this many colors around the color wheel.
// Fewer steps = the color only changes when you really turn the knob
// (a knob's reading always wiggles a little, even when you don't touch it).
int colorSteps = 24;

// The color step we last sent (-1 = nothing sent yet)
int lastColorStep = -1;

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
    Serial.println("DFPongController - Live Settings");
    Serial.println("================================");

    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);

    // Make every board read the knob as 0-1023
    // (ESP32 boards would read 0-4095 otherwise)
    analogReadResolution(10);

    controller.setControllerNumber(controllerNumber);
    controller.setStatusLED(LED_BUILTIN);

    // Settings that never change can still go here
    controller.setControllerName("Rainbow");

    // Set the first color from the knob's current position
    updateColorFromKnob();

    if (controller.begin())
    {
        Serial.println("Bluetooth initialized!");
        Serial.println("Turn the knob to change your paddle color.");
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
    // REQUIRED every loop - this is also what sends new settings to the game
    controller.update();

    // --------------------------------------------------------------------------
    // STEP 1: Change the paddle color when the knob moves
    // --------------------------------------------------------------------------
    updateColorFromKnob();

    // --------------------------------------------------------------------------
    // STEP 2: Move the paddle with the buttons
    // --------------------------------------------------------------------------
    if (digitalRead(upButtonPin) == LOW)
    {
        controller.sendControl(UP);
    }
    else if (digitalRead(downButtonPin) == LOW)
    {
        controller.sendControl(DOWN);
    }
    else
    {
        controller.sendControl(NEUTRAL);
    }
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

// Reads the knob and, if it moved to a new color step, sends the new color.
void updateColorFromKnob()
{
    int reading = analogRead(knobPin);                        // 0 - 1023
    int colorStep = map(reading, 0, 1024, 0, colorSteps);     // 0 - (colorSteps - 1)

    // Only send a new color when the step changed
    if (colorStep == lastColorStep)
    {
        return;
    }
    lastColorStep = colorStep;

    // Turn the step into a position on the color wheel (0 - 359 degrees)
    int hue = colorStep * 360 / colorSteps;

    int red, green, blue;
    hueToRGB(hue, red, green, blue);

    controller.setPlayerColor(red, green, blue);

    Serial.print("Hue: ");
    Serial.print(hue);
    Serial.print("  ->  ");
    Serial.println(controller.getSettingsJSON());
}

// Converts a hue (0-359 degrees on the color wheel) to bright red, green
// and blue values (0-255). The "&" means the function fills in the
// variables you give it.
//
//   0 = red, 60 = yellow, 120 = green, 180 = cyan, 240 = blue, 300 = magenta
void hueToRGB(int hue, int &red, int &green, int &blue)
{
    int section = hue / 60;                     // which sixth of the wheel (0-5)
    int rising = (hue % 60) * 255 / 60;         // goes 0 -> 255 inside the section
    int falling = 255 - rising;                 // goes 255 -> 0 inside the section

    if (section == 0)      { red = 255;     green = rising;  blue = 0;       }
    else if (section == 1) { red = falling; green = 255;     blue = 0;       }
    else if (section == 2) { red = 0;       green = 255;     blue = rising;  }
    else if (section == 3) { red = 0;       green = falling; blue = 255;     }
    else if (section == 4) { red = rising;  green = 0;       blue = 255;     }
    else                   { red = 255;     green = 0;       blue = falling; }
}
