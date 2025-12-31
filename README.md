# DFPongController

A beginner-friendly Arduino library for creating Bluetooth Low Energy (BLE) controllers that connect to the [DF Pong](https://digitalfuturesocadu.github.io/df-pong/) browser game.

![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)
![License](https://img.shields.io/badge/License-MIT-green)

## Supported Boards

| Board | Status | BLE Library |
|-------|--------|-------------|
| Arduino UNO R4 WiFi | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 IoT | Tested | ArduinoBLE (built-in) |
| Arduino Nano 33 BLE / BLE Sense | Supported | ArduinoBLE (built-in) |
| ESP32 / ESP32-S3 / ESP32-C3 | Supported | NimBLE-Arduino (install separately) |

**Note:** ESP32-S2 does not have Bluetooth and is not supported.

## Installation

### Step 1: Install the Library

#### Arduino Library Manager (Recommended)
1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries...**
3. Search for **"DFPongController"**
4. Click **Install**

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

ESP32 requires the **NimBLE-Arduino** library:

1. Go to **Sketch > Include Library > Manage Libraries...**
2. Search for **"NimBLE-Arduino"** by h2zero
3. Click **Install**

## Testing Your Controller

Once your code is uploaded:

1. Open the controller test page: **https://digitalfuturesocadu.github.io/df-pong/game/test/**
2. Select your controller number from the dropdown
3. Click **Connect**
4. Your controller should connect and show movement when you press buttons

## Quick Start

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
- Requires NimBLE-Arduino library (see installation above)
- Use `LED_BUILTIN` or specify your LED pin (varies by board)
- RSSI reading returns approximate value (-50 dBm) due to NimBLE limitations
- ESP32-S2 is NOT supported (no Bluetooth hardware)

## API Reference

### Setup Methods

| Method | Description |
|--------|-------------|
| `setControllerNumber(int n)` | **Required.** Set your unique number (1-242) |
| `setStatusLED(int pin)` | Set LED pin for connection status |
| `setDebug(bool enabled)` | Enable Serial debug messages |
| `begin()` | Initialize BLE with default name |
| `begin(const char* name)` | Initialize BLE with custom device name |

### Loop Methods

| Method | Description |
|--------|-------------|
| `update()` | **Required.** Call every `loop()` iteration |
| `sendControl(int direction)` | Send `UP`, `DOWN`, or `NEUTRAL` |

### Status Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `isConnected()` | `bool` | True if BLE connected |
| `isReady()` | `bool` | True if connected AND handshake complete |
| `getRSSI()` | `int` | Signal strength in dBm (-50 excellent, -90 poor) |
| `hasStrongSignal()` | `bool` | True if signal > -70 dBm |
| `getControllerNumber()` | `int` | Returns configured controller number |

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `UP` | 1 | Paddle moves up |
| `DOWN` | 2 | Paddle moves down |
| `NEUTRAL` | 0 | No movement |

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

### ESP32: NimBLE library not found
Make sure you installed the NimBLE-Arduino library:
1. **Sketch > Include Library > Manage Libraries...**
2. Search for **"NimBLE-Arduino"**
3. Install the library by h2zero

## Examples

- **StartTemplate** - Template with commented structure for creating your own controller
- **SimpleDigital** - Working example with two physical buttons

## Links

- [Test Your Controller](https://digitalfuturesocadu.github.io/df-pong/game/test/)
- [Play DF Pong](https://digitalfuturesocadu.github.io/df-pong/)
- [DF Pong Repository](https://github.com/DigitalFuturesOCADU/df-pong)
- [Digital Futures @ OCAD U](https://www.ocadu.ca/academics/undergraduate-studies/digital-futures)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Credits

Created by Digital Futures at OCAD University.
