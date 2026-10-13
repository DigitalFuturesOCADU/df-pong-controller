/*
 * 08_PositionControl - steer with a knob, taunt with a button
 * (needs the 2026 DF Pong game)
 *
 * Turn the knob and your paddle moves to that height. Press the button and
 * a word pops up beside your paddle.
 *
 * The paddle still moves at its normal speed, so a knob is smoother than
 * buttons, not faster. Older versions of the game ignore positions and
 * taunts: use UP / DOWN there (see 01_SimpleDigital).
 *
 * Wiring:
 *   Potentiometer: outer legs to 3.3V and GND, middle leg to A0
 *   Button:        one side to pin 2, the other to GND
 *
 * Any sensor that gives a range of readings works the same way: a slider,
 * a distance sensor, a light sensor, a flex sensor...
 */

#include <DFPongController.h>

DFPongController controller;

const int KNOB_PIN = A0;
const int TAUNT_BUTTON_PIN = 2;

// analogRead() gives 0-4095 on ESP32 boards and 0-1023 on most others
#if defined(ESP32)
const int KNOB_MAX = 4095;
#else
const int KNOB_MAX = 1023;
#endif

bool buttonWasDown = false;

void setup() {
    Serial.begin(9600);
    pinMode(TAUNT_BUTTON_PIN, INPUT_PULLUP);

    controller.setControllerNumber(1);   // CHANGE THIS to your controller number
    controller.begin();
}

void loop() {
    controller.update();

    // The knob sets the paddle's height: one end is the top, the other the bottom.
    // (To turn it around, swap 0 and KNOB_MAX.)
    int reading = analogRead(KNOB_PIN);
    controller.sendPosition(reading, 0, KNOB_MAX);

    // Taunt when the button is pressed (not while it's held)
    bool buttonDown = digitalRead(TAUNT_BUTTON_PIN) == LOW;
    if (buttonDown && !buttonWasDown) {
        controller.sendTaunt(2);   // 0 HA!  1 NICE  2 GG  3 OOPS  4 COME ON  5 WOW
    }
    buttonWasDown = buttonDown;

    delay(10);
}
