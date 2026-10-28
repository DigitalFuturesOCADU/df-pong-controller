/*
DF Pong Starter: Light

Moves your paddle to a height set by the light on an LDR:
in the dark the paddle goes to the top, in bright light to the bottom.

Pin A1 : LDR (the LDR circuit from Experiment 1, with a 10K resistor to GND)

Open the Serial Monitor at 9600 baud. Cover the LDR, then shine a light on it,
and put the two values you see in darkValue and brightValue.
Needs the DFPongController library (2.4 or newer) and NimBLE-Arduino, both from the Library Manager.
*/

#include <DFPongController.h>

DFPongController controller;

//your controller number (1 - 242): every player has a different one
int controllerNumber = 1;

//LDR variables
int ldrPin = A1;
int ldrValue;

//the readings in the dark and in bright light: change these to yours
int darkValue = 200;
int brightValue = 3200;

void setup()
{
  Serial.begin(9600);

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

  //read the LDR (0 - 4095) and print it, to find your dark and bright values
  ldrValue = analogRead(ldrPin);
  Serial.print("Light: ");
  Serial.println(ldrValue);

  //keep the value inside its range
  ldrValue = constrain(ldrValue, darkValue, brightValue);

  //darkValue puts the paddle at the top, brightValue at the bottom
  controller.sendPosition(ldrValue, darkValue, brightValue);
}
