/*
DF Pong Starter: Buttons

Moves your paddle with two buttons: hold one to move up, the other to move down.
Let go of both and the paddle stops.

Pin 32 : UP button (its other leg to GND)
Pin 33 : DOWN button (its other leg to GND)

INPUT_PULLUP: a button reads HIGH when it is not pressed and LOW when it is pressed,
so it needs no resistor.
Needs the DFPongController library (2.4 or newer) and NimBLE-Arduino, both from the Library Manager.
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

void setup()
{
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  //Customize your controller: remove the // at the start of a line to use it

  //your controller's name in the game (up to 20 characters)
  // controller.setControllerName("Thunderpaddle");

  //paddle color, as a web color code
  // controller.setPlayerColor("#ff7a1a");

  //paddle speed: 1 - 100 (normal is 10)
  // controller.setMoveSpeed(14);

  //paddle size: 50 - 200 percent (normal is 100)
  // controller.setPaddleSize(120);

  //sounds: a built-in name, or an https:// link straight to your own mp3 or wav file
  //built-in sounds: pingpong, claves, tick, bell, gong, bikebell, flourish, applause, pip, quindar,
  //clock, blip, boop, pluck, click, laser, drum, chirp, zap, thud, wood, glass, coin
  // controller.setHitSound("bikebell");    //when your paddle hits the puck
  // controller.setScoreSound("coin");      //when you score a point
  // controller.setWinSound("applause");    //when you win the match

  //background music: a built-in name, or an https:// link straight to your own mp3 file
  //built-in music: drift, tabletop, arcade, nightbus, gymnopedie, smallsteps, bounce, currents, palmleaf, spacemtv
  //(the Music tile in the game's lobby has to be set to "Players' choice" to hear it)
  // controller.setMusic("https://raw.githubusercontent.com/DigitalFuturesOCADU/df-pong/main/game/cc26/assets/music/palmleaf.mp3");

  controller.begin();
}

void loop()
{
  //keeps the connection to the game going: needed every loop
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
