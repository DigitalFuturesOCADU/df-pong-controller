/*
DF Pong Game Events 05: Your Own Sounds

Write your own tunes for the game's events, played on a piezo buzzer.

controller.setBuzzer(pin, false) keeps the controller's start / connect / drop sounds
but leaves the game events to you. controller.playTone(frequency, milliseconds) plays
one note without stopping loop(). For a tune, this sketch plays the next note when the
last one ends, so the paddle keeps moving while it plays.

A tune is a list of notes: { frequency in Hz, length in milliseconds }.
0 Hz is a rest. Some notes: C5 523, D5 587, E5 659, F5 698, G5 784, A5 880, B5 988, C6 1047

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

//Your tunes: { frequency, milliseconds }
const int scoredTune[][2] = { {784, 90}, {988, 90}, {1175, 90}, {1568, 220} };
const int concededTune[][2] = { {392, 150}, {0, 40}, {330, 300} };
const int matchPointTune[][2] = { {1047, 80}, {0, 80}, {1047, 80}, {0, 80}, {1047, 80} };
const int wonTune[][2] = { {523, 120}, {659, 120}, {784, 120}, {1047, 300}, {0, 80}, {988, 120}, {1047, 400} };
const int lostTune[][2] = { {659, 200}, {622, 200}, {587, 200}, {554, 500} };

//The tune that's playing
const int (*tune)[2] = nullptr;  //its notes
int tuneLength = 0;              //how many notes it has
int noteIndex = 0;               //which note is next
unsigned long nextNoteAt = 0;    //when to play it

//Start a tune (it replaces the one that's playing)
void playTune(const int notes[][2], int length)
{
  tune = notes;
  tuneLength = length;
  noteIndex = 0;
  nextNoteAt = millis();  //the first note plays now
}

//Plays the next note when it's time. Called every loop.
void updateTune()
{
  if (tune == nullptr || millis() < nextNoteAt)
  {
    return;
  }
  if (noteIndex >= tuneLength)
  {
    tune = nullptr;  //done
    return;
  }
  int frequency = tune[noteIndex][0];
  int ms = tune[noteIndex][1];
  controller.playTone(frequency, ms);
  nextNoteAt = millis() + ms + 15;  //a tiny gap between notes
  noteIndex = noteIndex + 1;
}

//This runs every time the game sends an event
void gameEvent(int event, int value)
{
  if (event == EVENT_HIT)
  {
    //a single blip: higher as the puck gets faster
    controller.playTone(600 + value * 4, 30);
  }
  else if (event == EVENT_COUNTDOWN)
  {
    if (value > 0)
    {
      controller.playTone(659, 80);   //3, 2, 1
    }
    else
    {
      controller.playTone(1319, 250); //GO
    }
  }
  else if (event == EVENT_SCORED)
  {
    playTune(scoredTune, 4);
  }
  else if (event == EVENT_CONCEDED)
  {
    playTune(concededTune, 3);
  }
  else if (event == EVENT_MATCH_POINT && value != 0)
  {
    playTune(matchPointTune, 5);  //only when you can win (1) or anyone can (2)
  }
  else if (event == EVENT_WON)
  {
    playTune(wonTune, 7);
  }
  else if (event == EVENT_LOST)
  {
    playTune(lostTune, 4);
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  //the buzzer, without the built-in game event sounds: those are yours now
  controller.setBuzzer(buzzerPin, false);

  //your function for every game event (above)
  controller.onGameEvent(gameEvent);

  controller.begin();
}

void loop()
{
  //keeps the connection going and calls gameEvent(): needed every loop
  controller.update();

  updateTune();

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
