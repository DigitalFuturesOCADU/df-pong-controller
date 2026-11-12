/*
DF Pong Game Events 01: Buzzer Sounds

The simplest way to use game events: one line, and a piezo buzzer plays a sound
for everything that happens in the game.

  - three notes when the controller starts
  - two rising notes when the game connects, two falling notes if it drops
  - a tick on every hit (higher as the puck speeds up), a jingle when you score,
    a warning at match point, a fanfare when you win... see the list at the bottom

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)
Pin 27 : passive piezo buzzer (its other leg to GND)

Written for the Adafruit Feather ESP32 V2. On a 2025 Nano controller, use pins 2 and 3
for the buttons and pin 11 for the buzzer.
Needs the 2026 game and the DFPongController library (2.5 or newer).
On ESP32 boards, also install NimBLE-Arduino from the Library Manager.
Experimental: game events may still change.
*/

#include <DFPongController.h>

DFPongController controller;

//your controller number (1 - 242): every player has a different one
int controllerNumber = 1;

//Button variables
int upButtonPin = 32;
int downButtonPin = 33;

//Buzzer pin
int buzzerPin = 27;

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  //built-in sounds on the buzzer. That's all it takes.
  //To keep only the start / connect / drop sounds, use: controller.setBuzzer(buzzerPin, false);
  controller.setBuzzer(buzzerPin);

  controller.begin();
}

void loop()
{
  //keeps the connection going and plays the buzzer: needed every loop
  controller.update();

  //read both buttons (LOW means pressed)
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

/*
What the buzzer plays:
  a match is starting   three rising notes
  countdown 3, 2, 1     a short beep each, then a higher, longer one on GO
  your paddle hits      a tick, higher as the puck speeds up
  you score             three rising notes
  they score            two falling notes
  match point           four notes: high if you can win, low if they can
  you win               a fanfare
  you lose              four falling notes
  pause / resume        one note / two rising notes

A sound never cuts off a more important one: a hit can't interrupt your winning fanfare.
*/
