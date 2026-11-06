# DFPongController

A beginner-friendly Arduino library for creating Bluetooth Low Energy (BLE) controllers that connect to the [DF Pong](https://github.com/DigitalFuturesOCADU/df-pong) browser game.

![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)
![License](https://img.shields.io/badge/License-MIT-green)

## What's New

| Version | What it added |
|---|---|
| **2.5** | *Experimental.* [A piezo buzzer and game events](#game-events-and-a-buzzer-experimental-25): your board can react to hits, points, match point, winning |
| **2.4** | `setControllerName()`: [name your controller](#player-settings) from the board. Pong 2026 starter examples. A blink pattern and a message when Bluetooth won't start |
| **2.3** | [`sendPosition()`](#position-a-knob): steer with a knob or any sensor with a range |
| **2.1** | [`setMusic()`](#music): background music for your matches |
| **2.0** | [Custom settings](#custom-settings): color, paddle speed and size, sounds, rule requests, live changes. No more handshake timeouts. Better ESP32 support. Every 1.x sketch still works |

Settings need the **2026 version** of the DF Pong game. Full notes: [releases](https://github.com/DigitalFuturesOCADU/df-pong-controller/releases).

**Contents**

- **Start here:** [Boards](#supported-boards) · [Browsers](#supported-browsers) · [Installation](#installation) · [Quick Start](#quick-start) · [Testing](#testing-your-controller) · [Controller number](#controller-number) · [LED patterns](#led-status-patterns)
- **Customize:** [Custom settings](#custom-settings) · [Built-in sounds](#sounds) · [Music](#music) · [Your own sound file](#your-own-sound-file)
- **On your board:** [Piezo buzzer](#sounds-on-the-controller-a-piezo-buzzer) · [Game events](#game-events-and-a-buzzer-experimental-25)
- **Reference:** [Examples](#examples) · [Troubleshooting](#troubleshooting) · [Board notes](#platform-specific-notes) · [API reference](#api-reference)

## Supported Boards

| Board | Status | BLE Library |
|-------|--------|-------------|
| Arduino UNO R4 WiFi | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 IoT | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 BLE / BLE Sense | Supported | ArduinoBLE (built-in) |
| ESP32 / ESP32-S3 / ESP32-C3 / Arduino Nano ESP32 / Adafruit Feather ESP32 V2 | Supported | NimBLE-Arduino **2.x** (install separately) |

**Note:** ESP32-S2 does not have Bluetooth and is not supported.

## Supported Browsers

The DF Pong game uses the [Web Bluetooth API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API), which has limited browser support.

| Browser | Desktop | Mobile |
|---------|---------|--------|
| Chrome | ✅ Supported | ✅ Android only |
| Edge | ✅ Supported | ✅ Android only |
| Opera | ✅ Supported | ✅ Android only |
| Safari | ❌ Not supported | ❌ Not supported |
| Firefox | ❌ Not supported | ❌ Not supported |
| iOS (all browsers) | ❌ Not supported | ❌ Not supported |

**Important Notes:**
- **iOS devices** (iPhone/iPad) do not support Web Bluetooth in any browser, including Chrome
- **Use Chrome on desktop** for the most reliable experience
- On Android, use Chrome, Edge, or Opera

## Installation

### Step 1: Install the Library

#### Arduino Library Manager (Recommended)
1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries...**
3. Search for **"DFPongController"**
4. Click **Install With Dependencies**

#### Manual Installation
1. Download this repository as a ZIP
2. In Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...**
3. Select the downloaded ZIP file

### Step 2: Install Board-Specific Dependencies

#### For Arduino Boards (UNO R4 WiFi, Nano 33 IoT, Nano 33 BLE)

No additional libraries needed - ArduinoBLE is included with the board package.

Just make sure you have the correct board package installed:
- **UNO R4 WiFi**: Install "Arduino UNO R4 Boards" via Boards Manager
- **Nano 33 IoT**: Install "Arduino SAMD Boards" via Boards Manager
- **Nano 33 BLE**: Install "Arduino Mbed OS Nano Boards" via Boards Manager

> **Nano 33 IoT: update the NINA firmware first.** The Nano 33 IoT's Bluetooth runs on a separate chip (the u-blox
> NINA module) with its own firmware. Many boards ship with firmware that's too old for Bluetooth: the controller then
> never shows up in the browser's list, and the Serial Monitor says
> `Please ensure your NINA firmware is version 3.0.0 or higher` and `ERROR: BLE failed to initialize!`.
> Update it once per board (it stays updated):
>
> 1. Plug the board in and close the Serial Monitor.
> 2. In Arduino IDE 2: **Tools → Firmware Updater**.
> 3. Choose the **Arduino NANO 33 IoT** and its port, click **Check Updates**, pick the newest **NINA** firmware
>    (3.0 or later) and click **Install**.
> 4. Upload your sketch again.
>
> If the board doesn't show up as a port, double-tap its reset button (the orange LED fades in and out) and try again.

#### For ESP32 Boards

ESP32 requires the **NimBLE-Arduino** library, **version 2.0 or newer**:

1. Go to **Sketch > Include Library > Manage Libraries...**
2. Search for **"NimBLE-Arduino"** by h2zero
3. Install (or update to) the latest **2.x** version

You also need an ESP32 board package: either **"esp32" by Espressif Systems** (for most ESP32 boards) or **"Arduino ESP32 Boards"** (for the Arduino Nano ESP32).

## Quick Start

There are **two ways to move your paddle**. Pick the one that suits your sensor:

| | **Direction**: up, down, stop | **Position**: a height |
|---|---|---|
| Good for | Buttons, switches, tilt, claps: anything on/off | Knobs, sliders, distance, light or flex sensors: anything with a range |
| You send | `sendControl(UP)`, `sendControl(DOWN)`, `sendControl(NEUTRAL)` | `sendPosition(reading, top, bottom)` |
| The paddle | Moves while you send UP or DOWN, stops on NEUTRAL | Moves to that height and stays there |
| Works in | Every game (2024–2026) | The 2026 game, library 2.3 or newer *(new)* |

The paddle moves at the same speed either way, so neither one is faster.

### Direction: two buttons

Each button goes from its pin to GND.

```cpp
#include <DFPongController.h>

DFPongController controller;

void setup() {
    pinMode(2, INPUT_PULLUP);  // UP button
    pinMode(3, INPUT_PULLUP);  // DOWN button
    
    // IMPORTANT: Set YOUR unique controller number (1-242)
    controller.setControllerNumber(1);  // <-- CHANGE THIS!
    
    controller.setStatusLED(LED_BUILTIN);
    controller.begin();
}

void loop() {
    controller.update();  // Required every loop!
    
    if (!digitalRead(2)) {
        controller.sendControl(UP);
    } else if (!digitalRead(3)) {
        controller.sendControl(DOWN);
    } else {
        controller.sendControl(NEUTRAL);
    }
}
```

### Position: a knob

A potentiometer: outer legs to 3.3V and GND, middle leg to A0.

```cpp
#include <DFPongController.h>

DFPongController controller;

void setup() {
    // IMPORTANT: Set YOUR unique controller number (1-242)
    controller.setControllerNumber(1);  // <-- CHANGE THIS!
    
    controller.setStatusLED(LED_BUILTIN);
    controller.begin();
}

void loop() {
    controller.update();  // Required every loop!
    
    // One end of the knob puts the paddle at the top, the other end at the bottom
    controller.sendPosition(analogRead(A0), 0, 1023);   // ESP32 boards: 0, 4095
}
```

- `sendPosition(reading, top, bottom)`: `top` is the reading that puts the paddle at the top, `bottom`
  the reading that puts it at the bottom. Any numbers work, so use the range your sensor really gives:

  ```cpp
  controller.sendPosition(analogRead(A0), 0, 1023);   // knob: 0 = top, 1023 = bottom
  controller.sendPosition(distanceCm, 5, 40);         // hand 5 cm away = top, 40 cm = bottom
  controller.sendPosition(lightLevel, 900, 100);      // bright (900) = top, dark (100) = bottom
  ```

  Swap the two numbers to turn the direction around. Readings past either end keep the paddle at that
  end. They're whole numbers (a reading of 12.7 counts as 12).
- Or send a fraction from `0.0` (top) to `1.0` (bottom): `controller.sendPosition(0.25)` is a quarter of
  the way down.
- Call `sendPosition()` every loop: the library only sends changes, at most every 20 ms.
- You can mix both: `sendControl(UP)` or `sendControl(DOWN)` switches back to direction (NEUTRAL
  doesn't), and the next `sendPosition()` switches to position again. `stopPosition()` also goes back
  to direction.
- Older games don't understand positions, so a position-only controller can't move there.
  *Experimental: positions may still change in a future version.*

The full sketch, with notes on wiring and ranges: **09_KnobSteering**.

## Testing Your Controller

Once your code is uploaded:

1. Open the controller test page: **https://digitalfuturesocadu.github.io/df-pong/game/cc26/?test=1**
   (it also shows your custom settings; the [2025 test page](https://digitalfuturesocadu.github.io/df-pong/game/test/) works too)
2. Select your controller number from the dropdown
3. Click **Connect**
4. Your controller should connect and show movement when you press buttons

## Controller Number

Each player/device needs a **unique** controller number between 1 and 242. This ensures devices can be identified correctly in the game.

```cpp
controller.setControllerNumber(1);  // Player 1
controller.setControllerNumber(2);  // Player 2
// ... up to 242
```

## LED Status Patterns

| Pattern | Meaning |
|---------|---------|
| Slow blink (500ms) | Disconnected, advertising |
| Fast blink (100ms) | Connected, handshaking |
| Solid ON | Ready to play |
| 3 quick blinks, then a pause | Bluetooth didn't start: open the Serial Monitor for what to do (see Troubleshooting) |

## Custom Settings

Add a few lines to `setup()` to make the game yours:

```cpp
controller.setControllerNumber(1);

controller.setControllerName("Thunderpaddle"); // your controller's name in the game
controller.setPlayerColor(208, 139, 91);   // paddle color (red, green, blue)
controller.setMoveSpeed(14);               // paddle speed (normal is 10)
controller.setPaddleSize(120);             // paddle size in percent (normal is 100)
controller.setHitSound("pluck");           // sound when your paddle hits the puck
controller.requestPointsToWin(7);          // ask to play to 7 points

controller.begin();
```

**Easiest way:** design your settings in the [Settings Builder](https://digitalfuturesocadu.github.io/df-pong/game/cc26/builder/).
It previews your paddle, tests your sound files the same way the game loads them, and gives you the code to paste.

- Settings can be set **before or after** `begin()`.
- If you change a setting **while connected**, the game picks it up automatically (see `07_LiveSettings`). The library sends at most 4 updates per second, and only when something actually changed.
- **During a match**, the host decides when your changes apply: right away, between points (the usual: colors, names and sounds at once, paddle speed and size at the next serve), or after the match. The game shows a message when a change has to wait. Rule requests and music always wait for the next match.
- Values that are out of range are fixed for you, and a message is printed in the Serial Monitor, for example:
  `DFPong: setMoveSpeed(500) is out of range (1-100), using 100`
- To see exactly what the game will receive: `Serial.println(controller.getSettingsJSON());`
- Older games (2025) ignore all settings - your controller still works there.

### Player Settings

These only change **your** side of the game. The game uses them unless the host has turned that kind of customization off.

| Function | Range | Game default |
|----------|-------|--------------|
| `setControllerName("Thunderpaddle")` | up to 20 characters (longer names are shortened) | name from the class list (see below) |
| `setPlayerColor(r, g, b)` | 0-255 each | off-white, for both players |
| `setPlayerColor("#d08b5b")` | `"#rrggbb"` or a web color name, like `"orange"` ([all 148 names](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/README.md#colors)) | off-white, for both players |
| `setMoveSpeed(speed)` | 1-100, whole numbers | 10 |
| `setPaddleSize(percent)` | 50-200, whole numbers | 100 |
| `setHitSound(nameOrUrl)` | [built-in sound name](#sounds) or [`https://` link](#your-own-sound-file) (max 200 characters) | `pingpong` |
| `setScoreSound(nameOrUrl)` | same as above | `bell` |
| `setWinSound(nameOrUrl)` | same as above | `flourish`, then applause |
| `setMusic(nameOrUrl)` | [music track name](#music) or `https://` link (max 200 characters) | the room's music |

Pass `""` to a name, color or sound function to go back to the default. Very dark colors are brightened a little by the game so you can still see your paddle.

**Names and the class list:** your class's list (`players-config.json`) has your name and can also give your
controller a name (`controllerName`). The game shows the first of these that exists:

1. the name your board sets with `setControllerName()`,
2. the controller name from the class list,
3. your own name from the class list.

So you can always name your controller from the board; without it, the class list decides. (The host can turn custom
names off, and then the class list is used.) Your own name still shows under the controller's name in the lobby, on
the VS screen and on the win screen. `setPlayerName()` is the same function under its older name (before 2.4).

### Game Setting Requests

These are shared by both players, so they are **requests**:

| Function | Range | Game default |
|----------|-------|--------------|
| `requestPointsToWin(points)` | 1-21, whole numbers | 10 |
| `requestPuckSpeed(speed)` | 1-20, in steps of 0.5 | 5 |
| `requestSpeedIncrement(increment)` | 0-1, in steps of 0.01 | 0.15 |

### How the Game Decides

Requests are settled once, when a match starts, and the result is shown on the **Match Rules** card before the countdown:

1. If the host **locked** a setting, the host's value is used.
2. If both players asked for **different** values, the game's conflict policy decides. The host's choices are **Meet in the middle** (the average, and the default), **Coin flip**, **Lower wins**, **Higher wins** and **Ignore requests**.
3. If only **one** player asked, that request is used.
4. If **nobody** asked, the game's normal value is used.

Requests that change during a match apply to the **next** match. Full details: [DF Pong protocol reference](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md).

### Any Setting (Advanced)

If a future version of the game adds a new setting before this library has a function for it, you can still use it by name:

```cpp
controller.setSetting("trailLength", 5);     // whole number
controller.setSetting("glow", 0.5);          // decimal number
controller.setSetting("rainbow", true);      // true / false
controller.setSetting("emoji", "rocket");    // text
controller.clearSetting("glow");             // remove one setting
controller.clearSettings();                  // remove all settings
```

Setting names may use letters, numbers, `_` and `-`. `setSetting()` does not check ranges - the game does. All settings together must fit in 512 bytes; if a new setting doesn't fit, it is not saved and a message is printed.

### Sounds

**Built-in sounds** ship with the game, so they always work. Any of them can be your hit, score or win sound. Upper or
lower case doesn't matter.

```cpp
controller.setHitSound("pluck");
```

| Name | Sound | Kind |
|---|---|---|
| `pingpong` | one table-tennis ball hit | Recording |
| `claves` | one strike of hardwood claves | Recording |
| `tick` | a watch tick | Recording |
| `bell` | a freshly cast bronze bell | Recording |
| `gong` | one strike of a cone gong | Recording |
| `bikebell` | a bicycle bell | Recording |
| `flourish` | a band's ruffle and flourish | Recording |
| `applause` | clapping and cheering | Recording |
| `pip` | a time-signal pip | Recording |
| `pipgo` | the time signal's final tone | Recording |
| `quindar` | an Apollo-era Quindar tone | Recording |
| `clock` | a kitchen-clock tick | Recording |
| `blip` | short square-wave beep | Made by the game |
| `boop` | low falling boop | Made by the game |
| `pluck` | plucked string | Made by the game |
| `click` | tiny click | Made by the game |
| `laser` | falling laser zap | Made by the game |
| `drum` | small drum | Made by the game |
| `chirp` | rising chirp | Made by the game |
| `zap` | electric zap | Made by the game |
| `thud` | soft thud | Made by the game |
| `wood` | wood block | Made by the game |
| `glass` | glass ping | Made by the game |
| `coin` | arcade coin | Made by the game |
| `classic` | the 2024 paddle hit | The 2024 originals |
| `wall` | the 2024 wall bounce | The 2024 originals |
| `ding` | the 2024 point ding | The 2024 originals |

Hear them all in the [Settings Builder](https://digitalfuturesocadu.github.io/df-pong/game/cc26/builder/).

**How your sounds play**, built-in or your own:

| Sound | Default | Longest | Notes |
|---|---|---|---|
| `setHitSound()` | `pingpong` | 1.2 s | Plays a little higher as the puck speeds up. Your next hit cuts off the last one |
| `setScoreSound()` | `bell` | 2.5 s | Starts 0.12 s after you score |
| `setWinSound()` | `flourish` | 8 s | Starts 0.3 s after you win, and applause joins in |

### Music

`setMusic()` picks the background music for your matches (new in 2.1):

```cpp
controller.setMusic("nightbus");   // or a link to your own music file
```

| Track | Mood | Kind |
|---|---|---|
| `drift` | calm | generated live by the game |
| `tabletop` | steady groove | generated |
| `arcade` | bright | generated |
| `nightbus` | lo-fi | generated |
| `gymnopedie` | calm piano (Satie) | recording, public domain |
| `smallsteps` | calm ambient (Lee Rosevere) | recording, CC BY 4.0 |
| `bounce` | minimal pulse (Loyalty Freak Music) | recording, CC0 |
| `currents` | lo-fi groove (HoliznaCC0) | recording, CC0 |
| `palmleaf` | light ragtime (Joplin) | recording, CC0 |
| `spacemtv` | light chiptune (Komiku) | recording, CC0 |

Sources and credits: the game's [music credits](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/game/cc26/assets/music/CREDITS.md).
If your opponent brings music too, the game swaps between your picks after each point. Whoever is at the
computer can also choose just one of them, any other track, or Off, on the lobby's Music tile.

**Your own music file:** use a link. It has the same rules as a sound link (below), except it can be larger than 2 MB,
because it's streamed, and it has to start within 10 seconds (3 minutes of mp3 at 128 kbps is about 3 MB). It loops, so
choose something that can start again from the top.

### Your own sound file

Give a link instead of a name:

```cpp
controller.setScoreSound("https://raw.githubusercontent.com/YOUR-NAME/YOUR-REPO/main/score.mp3");
```

**1. Make the sound:**

| | Use |
|---|---|
| Length | Hit: under 0.5 s. Score: 1–2 s. Win: 3–8 s |
| Format | **mp3** (128–192 kbps) or **wav**; ogg also works. Convert anything else, like a phone's m4a |
| File size | Under **2 MB** |
| Channels | **Mono**: the game places it on your side of the stereo itself |
| Start and end | Start right at the sound, and fade to silence at the end |
| Volume | Normalize to about −1 dB |
| File name | Short, lowercase, no spaces: `score.mp3` |

Your repository is public, so use sounds you made or have the rights to.

**2. Put it online:** upload it to a **public GitHub repository**.

**3. Copy its link:** open the file on GitHub, click **Raw**, and copy the address from your browser's address bar. It
starts with `https://raw.githubusercontent.com/...`. A GitHub Pages link (`https://YOUR-NAME.github.io/YOUR-REPO/score.mp3`) works
too. The page that shows the file (`github.com/.../blob/...`) **doesn't**, and neither do Google Drive, Dropbox, OneDrive
or iCloud links.

**4. Test it** in the [Settings Builder](https://digitalfuturesocadu.github.io/df-pong/game/cc26/builder/), which loads it
exactly the way the game does.

The rules for a link:
- It starts with **`https://`** (`http://` links are tried as `https://`).
- It's at most **200 characters**.
- The file loads within **8 seconds**.
- All your settings together fit in **512 bytes**. With links for all three sounds and your music, keep each link under about 100 characters.

If a sound can't be loaded, the game plays the default sound and shows why in the lobby.

The [full guide](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/README.md#make-your-own-sound-file) in the game's README has why each of these matters, the free tools to
use, what each error message means, and what to do when you replace a file.

## Sounds on the Controller (a piezo buzzer)

A piezo buzzer on your controller can give feedback: a jingle when it starts, a sound when the game connects,
a beep on every button press. Use Arduino's own `tone(pin, frequency, milliseconds)` in your sketch; it works
with every version of this library. Wire the piezo from a pin (the 2025 Nano controllers use **pin 11**) to GND.

```cpp
const int BUZZER_PIN = 11;
bool wasReady = false;
int lastDirection = NEUTRAL;

// Plays a few notes, one after another (this pauses loop() while it plays, so keep it short)
void jingle(const int* notes, int count, int ms) {
  for (int i = 0; i < count; i++) {
    tone(BUZZER_PIN, notes[i], ms);
    delay(ms + 20);
  }
  noTone(BUZZER_PIN);
}

void setup() {
  // ... pins, setControllerNumber(), etc.
  pinMode(BUZZER_PIN, OUTPUT);
  const int hello[] = {523, 659, 784};      // C E G: the controller is on
  jingle(hello, 3, 90);

  controller.begin();
}

void loop() {
  controller.update();

  // Two rising notes when the game has connected and is ready
  bool ready = controller.isReady();
  if (ready && !wasReady) {
    const int ok[] = {784, 1047};
    jingle(ok, 2, 80);
  }
  wasReady = ready;

  // read your buttons into direction (UP, DOWN or NEUTRAL), then:
  int direction = NEUTRAL;
  // ...
  controller.sendControl(direction);

  // A short beep when a button is pressed: high for UP, low for DOWN.
  // tone() with a length doesn't pause loop(), so this is fine every loop.
  if (direction != lastDirection && direction != NEUTRAL) {
    tone(BUZZER_PIN, direction == UP ? 1568 : 392, 40);
  }
  lastDirection = direction;
}
```

- Notes in Hz: C5 = 523, E5 = 659, G5 = 784, C6 = 1047, G6 = 1568, G4 = 392. Higher numbers are higher notes.
- `tone(pin, frequency, ms)` plays in the background; the `delay()` in `jingle()` is what waits for each note. Keep
  jingles short (under half a second) so they don't hold up the connection.
- A *passive* piezo plays any note. An *active* buzzer (it beeps on its own when powered) only makes one sound;
  use `digitalWrite()` for those.

## Game Events and a Buzzer (experimental, 2.5)

*Needs the 2026 game. Experimental: these may still change in a future version.*

The game tells your controller what just happened: your paddle hit the puck, you scored, it's match point,
you won. Your controller can play a sound, light an LED, buzz a motor or move a servo.

**A piezo buzzer, with built-in sounds:**

```cpp
controller.setBuzzer(11);   // in setup(): the buzzer's pin
```

That's all: it does what the `tone()` code in **Sounds on the Controller** above does by hand, and more. It plays a jingle when the controller starts, two rising notes when the game connects, two falling
notes if the connection drops, and a sound for every game event. Sounds never stop your `loop()`; a more
important sound (winning) is never cut off by a less important one (a hit). `setBuzzer(11, false)` keeps only
the start / connect / drop sounds, and `controller.playTone(880, 100)` plays your own note.

**Your own reaction to each event:**

```cpp
void gameEvent(int event, int value) {
    if (event == EVENT_SCORED) digitalWrite(ledPin, HIGH);
    if (event == EVENT_HIT) analogWrite(motorPin, value);   // value: how fast the puck was going
}

void setup() {
    // ...
    controller.onGameEvent(gameEvent);
}
```

Or ask in `loop()`: `int event = controller.readGameEvent();` (`NO_EVENT` when nothing happened), with
`controller.gameEventValue()` for its value.

| Event | When | `value` | With `setBuzzer()` |
|---|---|---|---|
| `EVENT_MATCH_START` | a match is starting (the NAME vs NAME screen) | | three notes |
| `EVENT_COUNTDOWN` | each beat of the countdown | 3, 2, 1, then 0 for GO | a short beep, higher and longer on GO |
| `EVENT_HIT` | your paddle hit the puck | puck speed, 0 (slow) to 255 (top speed) | a tick, higher as the puck speeds up |
| `EVENT_SCORED` | you scored | your score | three rising notes |
| `EVENT_CONCEDED` | the other player scored | their score | two falling notes |
| `EVENT_MATCH_POINT` | the next point can end the match | 1: you can win, 0: they can, 2: next point wins | four alternating notes: high if you can win, low if they can, high and low if both |
| `EVENT_WON` | you won | | a fanfare |
| `EVENT_LOST` | you lost | | four falling notes |
| `EVENT_PAUSED` | the game paused | | one note |
| `EVENT_RESUMED` | the game resumed | | two rising notes |

`controller.myScore()` and `controller.theirScore()` give the score as the game last told it. Older games
don't send events; everything else still works. See the **dfPongGameEvents** example.

## Examples

The examples are in two folders. Open them in Arduino IDE via **File > Examples > DFPongController**.

### Pong 2026

The starters for the 2026 game: simple sketches with every customization in `setup()`, commented out (remove the `//` to use one). Written for the Adafruit Feather ESP32 V2.

| Example | Description |
|---------|-------------|
| **dfPongStarterButtons** | Two buttons on pins 32 and 33 (`INPUT_PULLUP`, no resistors): hold one to move up, the other to move down |
| **dfPongStarterLight** | An LDR on pin A1 sets the paddle's height with `sendPosition()`: dark is the top, bright the bottom |
| **dfPongGameEvents** | Buttons plus a piezo buzzer: built-in sounds for every game event, and your own reactions (experimental, 2.5) |

### Pong 2025

Templates and other ways to control a paddle. Their buttons are on pins 2 and 3: on an ESP32, change them (see [ESP32 notes](#esp32)).

| Example | Description |
|---------|-------------|
| **00_StartTemplate** | Empty template with commented structure for creating your own custom controller |
| **01_SimpleDigital** | Basic two-button controller - paddle moves while buttons are held |
| **02_ClickDetection** | Tap-based control - paddle moves once per click instead of continuous hold |
| **03_ChordedInputs** | Use button combinations (chords) for multiple controls with fewer buttons |
| **04_GravityClick** | Flappy Bird-style control - click to go up, gravity pulls you down |
| **05_SlotMachine** | Chaotic controller - each button press sends a random direction for a random time |
| **06_CustomSettings** | Two-button controller with your own name, color, speed, size, sounds and a rule request |
| **07_LiveSettings** | Change your paddle color live with a potentiometer while you play |
| **09_KnobSteering** | Move your paddle with a knob: the simplest position example |

## Troubleshooting

### Nano 33 IoT: "BLE failed to initialize" / doesn't show up in the browser's list
The Serial Monitor shows `Please ensure your NINA firmware is version 3.0.0 or higher` and
`ERROR: BLE failed to initialize!`, and the status LED blinks 3 times, then pauses (library 2.4.3 and newer; older
versions leave it on). The board's Bluetooth chip needs a firmware update (once per board):

Follow the firmware steps in [Step 2](#step-2-install-board-specific-dependencies). You only need to do it once per board.

### "Call setControllerNumber before begin()"
You forgot to set your controller number. Add this before `begin()`:
```cpp
controller.setControllerNumber(YOUR_NUMBER);
```

### LED blinks but won't connect
1. Make sure the test page or game is open in your browser
2. Check that your controller number matches the dropdown selection
3. Try refreshing the browser page
4. Make sure Bluetooth is enabled on your computer/phone

### "Handshake timeout - disconnecting" / controller disconnects a few seconds after connecting
After connecting, the controller and the game say "hello" to each other (the handshake). If that doesn't finish, the controller disconnects so the real game can connect.
- **Library 1.x:** the hello was only sent from `sendControl()`. Sketches that only call `sendControl()` on a click had to be clicked within 5 seconds of connecting. **Update to 2.0** - `update()` now sends the hello by itself.
- Make sure `controller.update()` is called in **every** `loop()`, and that `loop()` doesn't get stuck in long `delay()`s.
- Make sure you are connecting from the DF Pong game or test page (other Bluetooth apps don't answer the hello).

### My settings don't show up in the game
1. Settings need the **2026** version of the game (older versions ignore them).
2. Check the Serial Monitor for messages starting with `DFPong:` - they explain values that were changed or refused.
3. Print what the game receives: `Serial.println(controller.getSettingsJSON());`
4. The host may have turned off that kind of customization, or locked a game setting.
5. Game requests (points to win, puck speed, speed increment) only apply when the **next match starts**.

### My custom sound doesn't play
- The link must start with `https://` and point directly to an `.mp3`, `.wav` or `.ogg` file.
- Open the link in your browser: it should play or download the sound, not show a web page.
- Use GitHub (Raw link) or GitHub Pages; Google Drive / Dropbox links don't work.
- Keep the file under 2 MB.
- The game's lobby shows why a sound couldn't be loaded. [What each message means](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/README.md#4-test-it-then-use-it).
- Replaced a file with the same name? GitHub can keep serving the old one for about 5 minutes.

### Weak or unstable connection
Use `getRSSI()` to check signal strength:
```cpp
if (controller.isConnected()) {
    Serial.print("Signal: ");
    Serial.print(controller.getRSSI());
    Serial.println(" dBm");
}
```
- `-30 to -50`: Excellent
- `-50 to -70`: Good
- `-70 to -90`: Weak (move closer)
- `< -90`: Very weak (connection may drop)

### ESP32: NimBLE library not found / compile errors in NimBLE
Make sure you installed the NimBLE-Arduino library, **version 2.x**:
1. **Sketch > Include Library > Manage Libraries...**
2. Search for **"NimBLE-Arduino"**
3. Install or update the library by h2zero to the latest 2.x version

### ESP32: "LED_BUILTIN was not declared" or no status LED
Some ESP32 boards don't have a built-in LED definition. Use the pin number of your LED instead, e.g. `controller.setStatusLED(2);` (see [ESP32 notes](#esp32)).

## Platform-Specific Notes

### Arduino UNO R4 WiFi
- Uses the built-in LED on pin `LED_BUILTIN`
- Tested and verified working

### Arduino Nano 33 IoT
- **Update the NINA firmware to the latest version before first use** (Tools → Firmware Updater; see
  [Step 2](#step-2-install-board-specific-dependencies)). With old firmware, Bluetooth doesn't start.
- Uses the built-in LED on pin `LED_BUILTIN`
- Button pins may need adjustment based on your wiring

### Arduino Nano 33 BLE / BLE Sense
- Same setup as Nano 33 IoT
- Built-in sensors can be used as controller inputs

### ESP32
- Requires **NimBLE-Arduino 2.x** (see installation above). Version 1.x of NimBLE-Arduino will not compile.
- ESP32-S2 is NOT supported (no Bluetooth hardware)
- **Arduino Nano ESP32:** works like the other Nano boards - same pin names, `LED_BUILTIN` works.
- **Generic ESP32 boards ("ESP32 Dev Module")** don't define `LED_BUILTIN`. The library then treats `LED_BUILTIN` as "no status LED" so the examples still compile. If your board has an LED (often on GPIO 2), use its number: `controller.setStatusLED(2);`
- **Pins on generic ESP32 boards:** the examples use pins 2 and 3 for buttons. On an "ESP32 Dev Module", GPIO 3 is the USB serial receive pin and GPIO 2 often has the LED, so wire your buttons to other pins (for example 18 and 19) and change the pin numbers in the sketch.
- Analog readings go from 0-4095 on ESP32 (0-1023 on most Arduino boards). Call `analogReadResolution(10);` in `setup()` to get 0-1023 everywhere.

## API Reference

### Setup Methods

| Method | Description |
|--------|-------------|
| `setControllerNumber(int n)` | **Required.** Set your unique number (1-242). Call before `begin()` |
| `setStatusLED(int pin)` | Set LED pin for connection status |
| `setDebug(bool enabled)` | Enable Serial debug messages |
| `begin()` | Initialize BLE with default name |
| `begin(const char* name)` | Initialize BLE with custom device name |

### Loop Methods

| Method | Description |
|--------|-------------|
| `update()` | **Required.** Call every `loop()` iteration. Also handles the handshake and sends changed settings |
| `sendControl(int direction)` | Send `UP`, `DOWN`, or `NEUTRAL` |
| `sendPosition(float position)` | *Experimental.* Steer to a height: 0.0 (top) to 1.0 (bottom) |
| `sendPosition(long reading, long top, long bottom)` | *Experimental.* Same, from a sensor reading: `top` and `bottom` are the readings for the top and the bottom, e.g. `(analogRead(A0), 0, 1023)` |
| `stopPosition()` | *Experimental.* Back to `UP` / `DOWN` |
| `actionsAvailable()` | True when the game listens for actions (the 2026 game) |

### Buzzer and Game Events (experimental, 2.5)

| Method | Description |
|--------|-------------|
| `setBuzzer(int pin, bool gameSounds = true)` | Built-in sounds on a piezo buzzer; `-1` for none |
| `playTone(int frequency, int ms)` | Your own note on the buzzer, without stopping `loop()` |
| `onGameEvent(function)` | Your function `(int event, int value)` runs for every game event |
| `readGameEvent()` / `gameEventValue()` | The next game event (`NO_EVENT` if none) and its value |
| `myScore()` / `theirScore()` | The score as the game last told it |

### Status Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `isConnected()` | `bool` | True if BLE connected |
| `isReady()` | `bool` | True if connected, subscribed AND handshake complete |
| `getRSSI()` | `int` | Signal strength in dBm (-50 excellent, -90 poor), 0 if not connected |
| `hasStrongSignal()` | `bool` | True if signal > -70 dBm (change with `setRSSIThreshold()`) |
| `setRSSIThreshold(int dBm)` | - | Threshold used by `hasStrongSignal()` |
| `getControllerNumber()` | `int` | Returns configured controller number |
| `getServiceUUID()` | `const char*` | The Bluetooth service UUID (for debugging) |

### Settings Methods (new in 2.0)

| Method | Description |
|--------|-------------|
| `setControllerName(name)` | Your controller's name in the game (max 20 characters). Comes first; without it the class list is used |
| `setPlayerName(name)` | Same as `setControllerName()` (its older name) |
| `setPlayerColor(r, g, b)` / `setPlayerColor("#rrggbb")` | Paddle color |
| `setMoveSpeed(speed)` | Paddle speed, 1-100 |
| `setPaddleSize(percent)` | Paddle size, 50-200 |
| `setHitSound(nameOrUrl)` | Sound when your paddle hits the puck |
| `setScoreSound(nameOrUrl)` | Sound when you score |
| `setWinSound(nameOrUrl)` | Sound when you win |
| `setMusic(nameOrUrl)` | Background music during your matches (2.1) |
| `requestPointsToWin(points)` | Request points to win, 1-21 |
| `requestPuckSpeed(speed)` | Request puck speed, 1-20 |
| `requestSpeedIncrement(increment)` | Request puck speed-up per hit, 0-1 |
| `setSetting(key, value)` | Any setting by name (number, decimal, true/false or text) |
| `clearSetting(key)` | Remove one setting |
| `clearSettings()` | Remove all settings |
| `getSettingsJSON()` | The settings exactly as the game reads them |

Name, color and sound functions also accept Arduino `String`s.

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `UP` | 1 | Paddle moves up |
| `DOWN` | 2 | Paddle moves down |
| `NEUTRAL` | 0 | No movement |

Game events have names too: see [the list](#game-events-and-a-buzzer-experimental-25).

### The DFPONG API

The ranges, limits and built-in names in this library come from the **DFPONG API**, the one spec the
game, this library and the Settings Builder all follow:
[reference](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/API.md) ·
[spec and how it changes](https://github.com/DigitalFuturesOCADU/df-pong/tree/main/api).

- `src/DFPongSpec.h` is generated from that spec. Don't edit it by hand.
- `DFPONG_API_VERSION` (e.g. `"1.0"`) says which version of the API this library follows; the
  constants are in the `DFPongSpec` namespace, for example `DFPongSpec::MOVE_SPEED_MAX`.
- A newer game may know sound or music names this library doesn't yet. They still work: the library
  just prints a note in the Serial Monitor.

## Links

- [Test Your Controller](https://digitalfuturesocadu.github.io/df-pong/game/cc26/?test=1)
- [Settings Builder](https://digitalfuturesocadu.github.io/df-pong/game/cc26/builder/)
- [Play DF Pong](https://digitalfuturesocadu.github.io/df-pong/)
- [DF Pong Repository](https://github.com/DigitalFuturesOCADU/df-pong)
- [DF Pong Protocol Reference](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md)
- [Digital Futures @ OCAD U](https://www.ocadu.ca/academics/undergraduate-studies/digital-futures)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Credits

Created by Digital Futures at OCAD University.
