/*
DF Pong: Game Events (experimental)

The game tells your controller what just happened: your paddle hit the puck,
you scored, it's match point, you won... This example plays a sound for each
one on a piezo buzzer, and shows how to react yourself (an LED, a motor...).

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)
Pin 11 : piezo buzzer (its other leg to GND). Any digital pin works: change buzzerPin.

Open the Serial Monitor at 9600 baud to see the events as they arrive.
Needs the 2026 game, the DFPongController library (2.5 or newer) and, on ESP32 boards,
NimBLE-Arduino, all from the Library Manager.
*/

#include <DFPongController.h>

DFPongController controller;

//your controller number (1 - 242): every player has a different one
int controllerNumber = 1;

//Button variables
int upButtonPin = 32;
int upButtonState;

int downButtonPin = 33;
int downButtonState;

//Buzzer pin
int buzzerPin = 11;

//This runs every time the game sends an event.
//event: what happened (EVENT_HIT, EVENT_SCORED, ...)
//value: a number that goes with it (see the list below)
void gameEvent(int event, int value)
{
  if (event == EVENT_MATCH_START)
  {
    Serial.println("A match is starting");
  }
  else if (event == EVENT_COUNTDOWN)
  {
    Serial.print("Countdown: ");
    Serial.println(value);  //3, 2, 1, then 0 for GO
  }
  else if (event == EVENT_HIT)
  {
    Serial.print("Hit! Puck speed: ");
    Serial.println(value);  //0 (slow) to 255 (top speed)
  }
  else if (event == EVENT_SCORED || event == EVENT_CONCEDED)
  {
    Serial.print(event == EVENT_SCORED ? "You scored! " : "They scored. ");
    Serial.print(controller.myScore());
    Serial.print(" - ");
    Serial.println(controller.theirScore());
  }
  else if (event == EVENT_MATCH_POINT)
  {
    //value: 1 = you can win with the next point, 0 = they can, 2 = whoever scores next wins
    Serial.println(value == 1 ? "Match point for you!" : value == 0 ? "Match point for them" : "Next point wins");
  }
  else if (event == EVENT_WON)
  {
    Serial.println("You won!");
  }
  else if (event == EVENT_LOST)
  {
    Serial.println("You lost");
  }
  else if (event == EVENT_PAUSED || event == EVENT_RESUMED)
  {
    Serial.println(event == EVENT_PAUSED ? "Paused" : "Playing again");
  }
  else if (event == EVENT_TAUNTED)
  {
    Serial.println("The other player taunted you");
  }

  //Your own reactions go here, for example an LED on pin 12 when you score:
  // if (event == EVENT_SCORED) digitalWrite(12, HIGH);
  // if (event == EVENT_MATCH_START) digitalWrite(12, LOW);
}

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  //built-in sounds on the buzzer: when it starts, connects or drops, and for every game event
  controller.setBuzzer(buzzerPin);

  //your own function for every game event (above)
  controller.onGameEvent(gameEvent);

  //your own notes: controller.playTone(frequency, milliseconds), e.g. playTone(880, 100)

  controller.begin();
}

void loop()
{
  //keeps the connection to the game going, plays the buzzer and handles events: needed every loop
  controller.update();

  //read both buttons (LOW means pressed)
  upButtonState = digitalRead(upButtonPin);
  downButtonState = digitalRead(downButtonPin);

  if (upButtonState == LOW)
  {
    controller.sendControl(UP);
  }
  else if (downButtonState == LOW)
  {
    controller.sendControl(DOWN);
  }
  else
  {
    controller.sendControl(NEUTRAL);
  }
}
