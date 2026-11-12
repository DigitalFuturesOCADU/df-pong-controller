/*
DF Pong Game Events 02: Event Light

An LED that reacts to the game, using your own function for game events:

  your paddle hits the puck   a quick flash
  you score                   three blinks
  match point                 stays on while the serve waits
  you win                     ten fast blinks
  you lose                    on for one second

The blinking never uses delay(): your event function only starts a pattern,
and loop() turns the LED on and off at the right times. That keeps the
paddle moving smoothly.

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)
Pin 15 : LED, through a 220 ohm resistor to GND

Written for the Adafruit Feather ESP32 V2. On a 2025 Nano controller, use pins 2 and 3
for the buttons and any free pin for the LED.
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

//LED variables
int ledPin = 15;
int blinksLeft = 0;            //how many times the LED still turns on
int blinkMs = 0;               //how long each on and each off lasts
bool ledOn = false;
unsigned long nextChange = 0;  //when the LED turns on or off next

//Start blinking: the LED turns on `times` times, `ms` milliseconds on and `ms` off
void blink(int times, int ms)
{
  blinksLeft = times;
  blinkMs = ms;
  ledOn = false;
  nextChange = millis();  //start now
}

//This runs every time the game sends an event.
//Keep it quick: start something here, and let loop() finish it.
void gameEvent(int event, int value)
{
  if (event == EVENT_HIT)
  {
    blink(1, 40);
  }
  else if (event == EVENT_SCORED)
  {
    blink(3, 120);
  }
  else if (event == EVENT_MATCH_POINT)
  {
    //on until the point is played (the next event changes it)
    blinksLeft = 0;
    ledOn = true;
    digitalWrite(ledPin, HIGH);
  }
  else if (event == EVENT_WON)
  {
    blink(10, 60);
  }
  else if (event == EVENT_LOST)
  {
    blink(1, 1000);
  }
  else if (event == EVENT_CONCEDED || event == EVENT_MATCH_START)
  {
    //turn the match point light off
    blinksLeft = 0;
    ledOn = false;
    digitalWrite(ledPin, LOW);
  }
}

//Turns the LED on and off for the blink pattern. Called every loop.
void updateLed()
{
  if (blinksLeft == 0 || millis() < nextChange)
  {
    return;
  }
  ledOn = !ledOn;
  digitalWrite(ledPin, ledOn ? HIGH : LOW);
  if (!ledOn)
  {
    blinksLeft = blinksLeft - 1;  //one blink done
  }
  nextChange = millis() + blinkMs;
}

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

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

  updateLed();

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
