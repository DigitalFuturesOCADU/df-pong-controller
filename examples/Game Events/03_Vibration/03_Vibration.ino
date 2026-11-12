/*
DF Pong Game Events 03: Vibration

A vibration motor you can feel in your hand:

  your paddle hits the puck   a short buzz, stronger when the puck is faster
  they score                  a long buzz
  you win                     a long, strong buzz

The event's value says how fast the puck was going: 0 (slow) to 255 (top speed).
analogWrite() turns that into the motor's strength.

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)
Pin 14 : vibration motor, through a transistor (a motor needs more current than a pin
         can give). For example an NPN transistor such as a 2N2222: pin 14 to its base
         through a 1k resistor, motor between 3V and the collector, emitter to GND,
         and a diode across the motor.

Written for the Adafruit Feather ESP32 V2. On a 2025 Nano controller, use pins 2 and 3
for the buttons and a PWM pin (marked ~) for the motor.
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

//Motor variables
int motorPin = 14;
unsigned long motorOffAt = 0;  //when to stop the motor (0: it's off)

//Run the motor at this strength (0 - 255) for this many milliseconds
void buzz(int strength, int ms)
{
  analogWrite(motorPin, strength);
  motorOffAt = millis() + ms;  //loop() stops it: no delay() needed
}

//This runs every time the game sends an event
void gameEvent(int event, int value)
{
  if (event == EVENT_HIT)
  {
    //slow puck: a gentle buzz, fast puck: a strong one
    int strength = map(value, 0, 255, 120, 255);
    buzz(strength, 70);
  }
  else if (event == EVENT_CONCEDED)
  {
    buzz(200, 300);
  }
  else if (event == EVENT_WON)
  {
    buzz(255, 800);
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  //your function for every game event (above)
  controller.onGameEvent(gameEvent);

  controller.begin();
}

void loop()
{
  //keeps the connection going and calls gameEvent(): needed every loop
  controller.update();

  //time to stop the motor?
  if (motorOffAt != 0 && millis() >= motorOffAt)
  {
    analogWrite(motorPin, 0);
    motorOffAt = 0;
  }

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
