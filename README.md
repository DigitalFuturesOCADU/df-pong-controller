# DFPongController

A beginner-friendly Arduino library for creating Bluetooth Low Energy (BLE) controllers that connect to the [DF Pong](https://github.com/DigitalFuturesOCADU/df-pong) browser game.

![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)
![License](https://img.shields.io/badge/License-MIT-green)

## What's New in 2.0

- **Custom settings** - give your player a name, a paddle color, a paddle speed and size, and your own sounds, right from your Arduino code. ([jump to Custom Settings](#custom-settings))
- **Game rule requests** - ask for the number of points to win, the puck speed and how fast it speeds up. The game combines both players' requests.
- **Live updates** - change settings while you play (for example, a knob that changes your paddle color).
- **No more handshake timeouts** - the library now finishes the connection handshake by itself inside `update()`. Controllers that only call `sendControl()` on a click (like `02_ClickDetection`) no longer disconnect after 5 seconds.
- **Better ESP32 support** - real signal strength (`getRSSI()`), correct transmit power, the device name shows up in the browser's device list, and more reliable connecting.
- **Fully backwards compatible** - every 1.x sketch works without changes, and a 2.0 controller still works with last year's (2025) game, which simply ignores the settings.

Settings need the **2026 version** of the DF Pong game. See the [protocol reference](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md) for the technical details.

## Supported Boards

| Board | Status | BLE Library |
|-------|--------|-------------|
| Arduino UNO R4 WiFi | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 IoT | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 BLE / BLE Sense | Supported | ArduinoBLE (built-in) |
| ESP32 / ESP32-S3 / ESP32-C3 / Arduino Nano ESP32 | Supported | NimBLE-Arduino **2.x** (install separately) |

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

#### For ESP32 Boards

ESP32 requires the **NimBLE-Arduino** library, **version 2.0 or newer**:

1. Go to **Sketch > Include Library > Manage Libraries...**
2. Search for **"NimBLE-Arduino"** by h2zero
3. Install (or update to) the latest **2.x** version

You also need an ESP32 board package: either **"esp32" by Espressif Systems** (for most ESP32 boards) or **"Arduino ESP32 Boards"** (for the Arduino Nano ESP32).

## Testing Your Controller

Once your code is uploaded:

1. Open the controller test page: **https://digitalfuturesocadu.github.io/df-pong/game/cc26/?test=1**
   (it also shows your custom settings; the [2025 test page](https://digitalfuturesocadu.github.io/df-pong/game/test/) works too)
2. Select your controller number from the dropdown
3. Click **Connect**
4. Your controller should connect and show movement when you press buttons

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

These only change **your** side of the game. The game uses them unless the teacher (admin) has turned that kind of customization off.

| Function | Range | Game default |
|----------|-------|--------------|
| `setControllerName("Thunderpaddle")` | up to 20 characters (longer names are shortened) | name from the class list (see below) |
| `setPlayerColor(r, g, b)` | 0-255 each | your side's color |
| `setPlayerColor("#d08b5b")` | any web color (`"#rrggbb"`, `"orange"`, ...) | your side's color |
| `setMoveSpeed(speed)` | 1-100 | 10 |
| `setPaddleSize(percent)` | 50-200 | 100 |
| `setHitSound(nameOrUrl)` | built-in sound name or `https://` link (max 200 characters) | built-in |
| `setScoreSound(nameOrUrl)` | same as above | built-in |
| `setWinSound(nameOrUrl)` | same as above | built-in |
| `setMusic(nameOrUrl)` | music track name or `https://` link (max 200 characters) | the room's music |

Pass `""` to a name, color or sound function to go back to the default. Very dark colors are brightened a little by the game so you can still see your paddle.

**Names and the class list:** your teacher's class list (`players-config.json`) has your name and can also give your
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
| `requestPointsToWin(points)` | 1-21 | 10 |
| `requestPuckSpeed(speed)` | 1-20 | 5 |
| `requestSpeedIncrement(increment)` | 0-1 | 0.15 |

### How the Game Decides

Requests are settled once, when a match starts, and the result is shown on the **Match Rules** card before the countdown:

1. If the teacher **locked** a setting, the teacher's value is used.
2. If both players asked for **different** values, the game's conflict policy decides. By default it uses the **average** (other policies: a random pick of the two, the lower one, the higher one, or ignore requests).
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

**Built-in sounds** ship with the game, so they always work:

| Kind | Names |
|---|---|
| Recordings (from public archives) | `pingpong`, `claves`, `tick`, `bell`, `gong`, `bikebell`, `flourish`, `applause`, `pip`, `pipgo`, `quindar`, `clock` |
| Synthesized by the game | `blip`, `boop`, `pluck`, `click`, `laser`, `drum`, `chirp`, `zap`, `thud`, `wood`, `glass`, `coin` |
| The 2024 originals | `classic`, `wall`, `ding` |

Try them all in the [Settings Builder](https://digitalfuturesocadu.github.io/df-pong/game/cc26/builder/). The game's
[sound list](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/game/cc26/assets/sounds/manifest.json)
is the full, current list.

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
computer can also choose just one of them, or any other track, on the lobby's Music tile. A music link has the
same rules as a sound link (below), except it may be long and larger than 2 MB, because it's streamed.

### Your own sound file

**Your own sound file** can be used by giving a link instead of a name:

```cpp
controller.setScoreSound("https://raw.githubusercontent.com/YOUR-NAME/YOUR-REPO/main/score.mp3");
```

Rules for sound links:
- The link must start with **`https://`** (`http://` links are tried as `https://`).
- It must point **directly to the file** (`.mp3` or `.wav`), not to a web page that shows the file.
- Keep sounds **short** and the file **under 2 MB**. It has to load within 8 seconds.
- The link can be at most **200 characters**.
- If the sound can't be loaded, the game plays the default sound and shows why in the lobby.

Where to host your file (free, and the browser is allowed to load from them):
1. **A public GitHub repository** - upload the file, open it on GitHub, click **Raw**, and copy the link. It starts with `https://raw.githubusercontent.com/...`
2. **GitHub Pages** - turn on Pages in your repository's settings. The link looks like `https://YOUR-NAME.github.io/YOUR-REPO/score.mp3`

Links from Google Drive, Dropbox, OneDrive or iCloud **don't work** - the browser isn't allowed to load sounds from them.

## In Game Actions (experimental)

*Needs the 2026 game. Experimental: these may still change in a future version.*

Settings describe your controller; **In Game Actions** are things you do during play, sent the moment
they happen.

```cpp
controller.sendTaunt(2);   // "GG" pops up beside your paddle
```

`sendTaunt(n)` shows a word beside your paddle with a ring and a sound (your own hit sound, if you set one):

| `n` | 0 | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|---|
| Word | HA! | NICE | GG | OOPS | COME ON | WOW |

- The game shows at most one taunt every 2 seconds per player and ignores the rest.
- The host can turn taunts off with the **In Game Actions** switch.
- `actionsAvailable()` is true when the game is listening. Older games ignore In Game Actions, and your
  controller still plays there.

Steering with a knob is one of the two ways to move your paddle: see [Quick Start](#quick-start).
`08_PositionControl` has both: a knob for steering and a button for taunts.

## Platform-Specific Notes

### Arduino UNO R4 WiFi
- Uses the built-in LED on pin `LED_BUILTIN`
- Tested and verified working

### Arduino Nano 33 IoT
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
| `sendTaunt(int emote)` | *Experimental.* A word beside your paddle: 0 HA!, 1 NICE, 2 GG, 3 OOPS, 4 COME ON, 5 WOW |
| `actionsAvailable()` | True when the game listens for actions (the 2026 game) |

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

## LED Status Patterns

| Pattern | Meaning |
|---------|---------|
| Slow blink (500ms) | Disconnected, advertising |
| Fast blink (100ms) | Connected, handshaking |
| Solid ON | Ready to play |

## Controller Number

Each player/device needs a **unique** controller number between 1 and 242. This ensures devices can be identified correctly in the game.

```cpp
controller.setControllerNumber(1);  // Player 1
controller.setControllerNumber(2);  // Player 2
// ... up to 242
```

## Troubleshooting

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
4. The teacher (admin) may have turned off that kind of customization, or locked a game setting.
5. Game requests (points to win, puck speed, speed increment) only apply when the **next match starts**.

### My custom sound doesn't play
- The link must start with `https://` and point directly to an `.mp3` or `.wav` file.
- Open the link in your browser: it should play or download the sound, not show a web page.
- Use GitHub (Raw link) or GitHub Pages; Google Drive / Dropbox links don't work.
- Keep the file under 2 MB. The game lobby shows why a sound couldn't be loaded.

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

## Examples

The examples are in two folders. Open them in Arduino IDE via **File > Examples > DFPongController**.

### Pong 2026

The starters for the 2026 game: simple sketches with every customization in `setup()`, commented out (remove the `//` to use one). Written for the Adafruit Feather ESP32 V2.

| Example | Description |
|---------|-------------|
| **dfPongStarterButtons** | Two buttons on pins 32 and 33 (`INPUT_PULLUP`, no resistors): hold one to move up, the other to move down |
| **dfPongStarterLight** | An LDR on pin A1 sets the paddle's height with `sendPosition()`: dark is the top, bright the bottom |

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
| **08_PositionControl** | Steer with a knob and taunt with a button (experimental actions) |
| **09_KnobSteering** | Move your paddle with a knob: the simplest position example |

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
