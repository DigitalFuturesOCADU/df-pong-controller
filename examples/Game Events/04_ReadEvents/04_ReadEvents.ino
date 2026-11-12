/*
DF Pong Game Events 04: Read Events

Another way to get game events: instead of a function that the library calls,
ask for them in loop() with controller.readGameEvent().

This example prints every event and the score in the Serial Monitor (9600 baud).
It's a good way to see what your controller receives, and when.

Use readGameEvent() OR onGameEvent(), not both: once onGameEvent() is set,
readGameEvent() always returns NO_EVENT.

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)

Written for the Adafruit Feather ESP32 V2. On a 2025 Nano controller, use pins 2 and 3
for the buttons.
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

//Prints one event and the number that came with it
void printEvent(int event, int value)
{
  if (event == EVENT_MATCH_START)
  {
    Serial.println("A match is starting");
  }
  else if (event == EVENT_COUNTDOWN)
  {
    if (value > 0)
    {
      Serial.println(value);  //3, 2, 1
    }
    else
    {
      Serial.println("GO!");  //0
    }
  }
  else if (event == EVENT_HIT)
  {
    Serial.print("Hit! Puck speed (0 - 255): ");
    Serial.println(value);
  }
  else if (event == EVENT_SCORED || event == EVENT_CONCEDED)
  {
    if (event == EVENT_SCORED)
    {
      Serial.print("You scored! ");
    }
    else
    {
      Serial.print("They scored. ");
    }
    //the library keeps the score for you
    Serial.print(controller.myScore());
    Serial.print(" - ");
    Serial.println(controller.theirScore());
  }
  else if (event == EVENT_MATCH_POINT)
  {
    if (value == 1)
    {
      Serial.println("Match point: you win with the next point");
    }
    else if (value == 0)
    {
      Serial.println("Match point: they win with the next point");
    }
    else
    {
      Serial.println("Match point: whoever scores next wins");  //2
    }
  }
  else if (event == EVENT_WON)
  {
    Serial.println("You won!");
  }
  else if (event == EVENT_LOST)
  {
    Serial.println("You lost");
  }
  else if (event == EVENT_PAUSED)
  {
    Serial.println("Paused");
  }
  else if (event == EVENT_RESUMED)
  {
    Serial.println("Playing again");
  }
  else
  {
    //a newer game may send events this library doesn't know yet
    Serial.print("Event ");
    Serial.print(event);
    Serial.print(", value ");
    Serial.println(value);
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  controller.begin();
}

void loop()
{
  //keeps the connection going and collects events: needed every loop
  controller.update();

  //read every event that has arrived (up to 7 wait; after that the oldest are dropped)
  int event = controller.readGameEvent();
  while (event != NO_EVENT)
  {
    int value = controller.gameEventValue();
    printEvent(event, value);
    event = controller.readGameEvent();
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
