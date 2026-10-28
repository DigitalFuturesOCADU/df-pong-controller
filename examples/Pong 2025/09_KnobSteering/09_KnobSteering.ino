/*
 * 09_KnobSteering - move your paddle with a knob
 * (needs the 2026 DF Pong game)
 *
 * There are two ways to move your paddle:
 *   - Direction: send UP, DOWN or NEUTRAL (buttons, tilt...): see 01_SimpleDigital
 *   - Position:  send a height (knobs, sliders, sensors with a range): this example
 *
 * Turn the knob and the paddle moves to that height: one end of the knob is
 * the top of the screen, the other end is the bottom. The paddle moves at its
 * normal speed, so a knob is smoother than buttons, not faster.
 *
 * Wiring (a potentiometer):
 *   one outer leg  -> 3.3V
 *   other outer leg -> GND
 *   middle leg     -> A0
 *
 * Older versions of the game don't understand positions: use buttons there.
 */

#include <DFPongController.h>

DFPongController controller;

const int KNOB_PIN = A0;

// The readings at the two ends of the knob.
// analogRead() gives 0-4095 on ESP32 boards and 0-1023 on most others.
// Swap them to turn the direction around. With another sensor, use the
// readings it really gives at each end (for example 80 and 600).
#if defined(ESP32)
const int TOP_READING = 0;
const int BOTTOM_READING = 4095;
#else
const int TOP_READING = 0;
const int BOTTOM_READING = 1023;
#endif

void setup() {
    Serial.begin(9600);

    controller.setControllerNumber(1);   // CHANGE THIS to your controller number
    controller.setStatusLED(LED_BUILTIN);
    controller.begin();
}

void loop() {
    controller.update();   // required every loop

    // Read the knob and send it as a position.
    // Call this every loop: the library only sends real changes.
    int reading = analogRead(KNOB_PIN);
    controller.sendPosition(reading, TOP_READING, BOTTOM_READING);

    delay(10);
}
