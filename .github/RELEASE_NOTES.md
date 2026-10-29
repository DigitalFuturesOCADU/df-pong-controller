# Release Notes

## v2.5.0 - Game events and a buzzer (experimental, not released yet)

Needs the 2026 game (DFPONG API 1.3); older games don't send events and every sketch works as before.

- New events characteristic (`19b10014-...` + your controller's suffix): the game tells your controller
  what just happened: match start, countdown, your hit, you scored, they scored, match point, won, lost,
  paused, resumed, taunted.
- `setBuzzer(pin)`: built-in sounds on a piezo buzzer for starting, connecting, dropping and every game
  event, without stopping `loop()`. `playTone(frequency, ms)` for your own notes.
- `onGameEvent(function)` or `readGameEvent()` / `gameEventValue()`: react yourself (LEDs, motors, servos).
  `myScore()` / `theirScore()`.
- New example **dfPongGameEvents** (Pong 2026).
- Host tests in `extras/host-tests` (run without a board).

## v2.4.3 - Says why when Bluetooth won't start

- If Bluetooth can't start (most often a Nano 33 IoT with old NINA firmware), the status LED now blinks
  **3 times, then pauses**, instead of staying on as if everything were fine.
- The Serial Monitor explains what to do, for your board (Nano 33 IoT and UNO R4 WiFi: update the
  firmware with Tools > Firmware Updater). The message repeats every 10 seconds, so you see it even if
  you open the Serial Monitor late.

## v2.4.2 - Pong 2026 starters

- Examples are now in two folders: **Pong 2026** and **Pong 2025** (File > Examples > DFPongController).
- New in Pong 2026: **dfPongStarterButtons** (two buttons on pins 32 and 33) and **dfPongStarterLight**
  (an LDR on A1 sets the paddle's height with `sendPosition()`). Both list every customization in
  `setup()`, commented out. Written for the Adafruit Feather ESP32 V2.
- The earlier examples, 00_StartTemplate to 09_KnobSteering, are unchanged in Pong 2025.

## v2.4.1

- `sendPosition(reading, top, bottom)`: the two numbers are now called `top` and `bottom` (the readings
  that put the paddle at the top and at the bottom), in the code, the warning message and the README.
  The README shows a knob, a distance sensor and a reversed light sensor. No change in behaviour.

## v2.4.0 - Name your controller from the board

- New `setControllerName("Thunderpaddle")`: your controller's name in the game. With the 2026 game
  (DFPONG API 1.2) the name set on the board now comes first; without it the game uses the controller
  name from the class list, then your own name.
- `setPlayerName()` still works and does the same thing (it was the function's name before 2.4).
- Examples use `setControllerName()`.

## v2.3.1

- New example **09_KnobSteering**: move your paddle with a knob, nothing else. Position is now
  presented as one of the two ways to move a paddle (direction with buttons, or position with a knob).
- README: Quick Start shows both ways, with an example for each.

## v2.3.0 - Actions (experimental)

Needs the 2026 game; older games ignore actions and every sketch works as before.

- New actions characteristic (`19b10013-...` + your controller's suffix, DFPONG API 1.1) for things
  that happen during play.
- `sendPosition(position)` / `sendPosition(value, low, high)`: steer with a knob, slider or distance sensor.
  The paddle moves to that height at its normal speed. `stopPosition()` (or UP / DOWN) goes back to buttons.
- `sendTaunt(emote)`: a word pops up beside your paddle (the game allows one every 2 seconds).
- `actionsAvailable()`: true when the game listens for actions.
- New example **08_PositionControl**.

## v2.2.0 - The DFPONG API

- Ranges, limits, built-in sound and music names, Bluetooth UUIDs and timing now come from
  `src/DFPongSpec.h`, generated from the DFPONG API spec the game also follows
  ([reference](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/API.md)). The library and
  the game can no longer disagree about them.
- New: `DFPONG_API_VERSION` (`"1.0"`) and the `DFPongSpec` constants, e.g. `DFPongSpec::PADDLE_SIZE_MAX`.
- No change to how controllers behave; every sketch works as before.

## v2.1.1

- `setMusic()` knows the game's six recorded tracks (`gymnopedie`, `smallsteps`, `bounce`, `currents`,
  `palmleaf`, `spacemtv`), so choosing one no longer prints a warning.

## v2.1.0 - Background Music

- New `setMusic()`: pick the background music for your matches, by name (`drift`, `tabletop`, `arcade`,
  `nightbus`) or with an `https://` link. If both players bring music, the game swaps between their picks
  each point, or the host chooses one.
- The list of built-in sound names now includes the game's new recordings (`pingpong`, `claves`, `tick`,
  `bell`, `gong`, `bikebell`, `flourish`, `applause`, `pip`, `pipgo`, `quindar`, `clock`) and `wall`/`ding`,
  so using them no longer prints a warning. Names are now matched without regard to upper/lower case,
  like the game does.
- Example 06_CustomSettings shows `setMusic()`.

## v2.0.0 - Custom Settings

Works with the 2026 DF Pong game, and still works with the 2025 game and every 1.x sketch (no code changes needed).

### New: Custom Settings
- New config characteristic (`19b10012-...` + your controller's suffix, READ | NOTIFY, JSON up to 512 bytes) that the 2026 game reads. Older games ignore it.
- Player settings: `setPlayerName()`, `setPlayerColor()`, `setMoveSpeed()`, `setPaddleSize()`, `setHitSound()`, `setScoreSound()`, `setWinSound()`
- Game rule requests (negotiated with the other player by the game): `requestPointsToWin()`, `requestPuckSpeed()`, `requestSpeedIncrement()`
- Any setting by name, for settings future games add: `setSetting()`, `clearSetting()`, `clearSettings()`, `getSettingsJSON()`
- Settings can be set before or after `begin()`; changes made while connected are sent automatically (at most every 250 ms)
- Out-of-range values are clamped, with a friendly warning on the Serial Monitor
- Name, color and sound functions also accept Arduino `String`s

### Fixes
- The connection handshake is now driven by `update()` (sent every 500 ms until the game answers). Sketches that only call `sendControl()` on clicks no longer get disconnected after 5 seconds.
- ESP32: the handshake was sent before the game subscribed and was then never repeated. The library now tracks subscriptions (`isReady()` also requires one, like on Arduino boards).
- ESP32: empty writes are ignored instead of reading a missing byte (Arduino boards: an empty write could no longer complete the handshake by mistake).
- ESP32: transmit power is now really +9 dBm (`setPower(9)`; NimBLE 2.x takes dBm, not a power-level enum).
- ESP32: `getRSSI()` returns the real signal strength instead of a fixed -50.
- ESP32: the device name is advertised again (scan response), so it shows in the browser's device list.
- ESP32: the handshake timeout now disconnects the right connection.
- A movement value held back by the 20 ms rate limit is now sent by `update()` instead of being lost (e.g. a quick second click).
- Calling `begin()` twice no longer adds a second copy of the Bluetooth service.
- `setControllerNumber()` after `begin()` is ignored with a warning (the UUIDs can't change anymore); `getServiceUUID()` works right after `setControllerNumber()`.
- The handshake timeout waits up to 10 s for the game to subscribe (slow service discovery), then 5 s for the handshake.
- On ESP32 boards without `LED_BUILTIN` (e.g. "ESP32 Dev Module") the examples now compile; there is no status LED unless you pass a pin number.
- Arduino boards: settings are padded with trailing spaces so their length never lands exactly on a Bluetooth packet boundary. ArduinoBLE answers a read at the exact end of a value with an error instead of an empty reply, which could make the game fail to load some settings.

### Examples
- **06_CustomSettings** - name, color, speed, size, sounds and a rule request
- **07_LiveSettings** - change the paddle color live with a potentiometer
- **00_StartTemplate** - new optional custom settings block
- **02_ClickDetection** - notes that no click is needed to connect anymore

### Requirements
- ESP32 needs NimBLE-Arduino 2.x

## v1.0.0 - Initial Release

Initial release of the DFPongController Arduino library.

### Features
- BLE controller library for the DF Pong browser game
- Beginner-friendly API (no pointers required)
- LED status indicators for connection state
- RSSI signal strength monitoring

### Supported Boards
- Arduino UNO R4 WiFi
- Arduino Nano 33 IoT  
- Arduino Nano 33 BLE / BLE Sense
- ESP32 (requires NimBLE-Arduino library)

### Examples
- **StartTemplate** - Template with commented structure for creating your own controller
- **SimpleDigital** - Working example with two physical buttons

### Links
- [Test Your Controller](https://digitalfuturesocadu.github.io/df-pong/game/test/)
- [Play DF Pong](https://digitalfuturesocadu.github.io/df-pong/)
