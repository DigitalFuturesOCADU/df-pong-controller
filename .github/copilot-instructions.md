# GitHub Copilot Instructions

## Project Overview

Arduino library for BLE controllers that connect to the [DF Pong](https://github.com/DigitalFuturesOCADU/df-pong) browser game. Supports Arduino Nano 33 IoT, Nano 33 BLE, UNO R4 WiFi, and ESP32 (NimBLE-Arduino 2.x).

The wire protocol is defined in the game repo: [docs/PROTOCOL.md](https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md). That document is authoritative. **Golden rule:** every controller generation must keep working - only ever add next to the old behaviour, never change its meaning.

## Architecture

### Platform Abstraction Pattern
The library uses compile-time platform detection via `#ifdef DFPONG_USE_NIMBLE`:
- **ESP32**: Uses NimBLE-Arduino 2.x with callback classes (`ServerCallbacks`, `CharacteristicCallbacks` with `onWrite` + `onSubscribe`)
- **Arduino boards**: Uses ArduinoBLE with static event handlers (`onBLEConnected`, etc.)

All platform-specific code is isolated using `#ifdef`/`#else`/`#endif` blocks. When modifying BLE functionality, update BOTH implementations.

### Threading Model
- **ArduinoBLE**: event handlers run inside `BLE.poll()`, i.e. inside `update()` - single threaded.
- **NimBLE (ESP32)**: callbacks run in the NimBLE host task, concurrently with `loop()`. Callbacks only write the `volatile` fields (`_deviceConnected`, `_subscribed`, `_handshakeComplete`, `_connHandle`, `_connectionEvents`). Everything else (`_lastSentValue`, `_desiredValue`, timers, settings) is only touched from `loop()` via `update()` / `sendControl()` / setters. `update()` notices a changed `_connectionEvents` counter and calls `resetState()`.

### Singleton Pattern
`DFPongController::_instance` provides static callback access. Only one controller instance is supported per device.

### UUID Generation
Controller numbers (1-242) generate unique BLE UUIDs via `generateUUIDs()`. UUIDs must match the JavaScript game exactly:
```cpp
// suffix = hex(13 + controllerNumber): controller 1 -> "0e", controller 2 -> "0f"
// Service:        "19b10010-e8f2-537e-4f6c-d104768a12" + suffix
// Movement char:  "19b10011-e8f2-537e-4f6c-d104768a12" + suffix  (1 byte, READ|WRITE|NOTIFY - never change)
// Config char:    "19b10012-e8f2-537e-4f6c-d104768a12" + suffix  (JSON <= 512 bytes, READ|NOTIFY, 2026+)
```
The service UUID must stay in the advertising packet - the browser filters on it.

### Connection Flow
1. `begin()` → advertise the service UUID
2. Central connects and subscribes to the movement characteristic
3. `update()` notifies `HANDSHAKE` (3) immediately, then every 500 ms, while connected + subscribed + not complete
4. Central writes `3` → `_handshakeComplete = true` (empty writes are ignored)
5. `sendControl()` / auto-NEUTRAL now send real values (rate limited to one per 20 ms; `update()` flushes a held-back value)
6. Timeouts: disconnect if not subscribed within 10 s of connecting, or no handshake within 5 s of subscribing

Note: ArduinoBLE echoes a central's write as a notification when the central is subscribed (that's ArduinoBLE behaviour, not ours), so the game will see its own `3` come back.

### Settings (config characteristic)
- Stored as one JSON buffer (`_settingsJson`, 512 bytes + NUL) that is edited in place: `{"v":1,"key":value,...}`. `"v"` is always first and can't be changed.
- `storeSetting()` adds/replaces an entry, `removeSetting()` deletes one, `findSetting()` parses only our own output format. Keys are restricted to `[A-Za-z0-9_-]` (max 32) so they never need escaping; text values are escaped by `emitEntry()`.
- Setting an identical value is a no-op (no notification). A setting that would push the JSON past 512 bytes is refused with a warning.
- Changes after `begin()` set `_settingsDirty`; `update()` pushes the value + a notification at most every 250 ms. The game ignores the notification payload and re-reads the characteristic.
- Numbers are formatted without `printf("%f")` (`formatDecimal()`, up to 4 decimals) because float printf is missing or large on some cores.
- No heap allocation or `String` concatenation in `update()` / `sendControl()`.

## Beginner-Friendly API Guidelines

- **No pointers/references in public API** - Use simple `int`, `float`, `bool`, `const char*` (plus `const String&` overloads where beginners pass Strings)
- **Method chaining setup**: `setControllerNumber()` → `setStatusLED()` → (settings) → `begin()`
- **Mandatory loop call**: `update()` must be called every `loop()` iteration
- **Direction constants**: Always expose `UP`, `DOWN`, `NEUTRAL` as global constants
- **Warnings, not silence**: out-of-range or refused settings print a `DFPong: ...` line to Serial even without debug mode, explaining what was used instead
- **setSetting overloads** must stay unambiguous for literals (`5`, `5u`, `5L`, `0.5`, `0.5f`, `true`, `"text"`, `millis()`); test new overloads with a compile check

## Code Style

- Use descriptive section comments: `// ============================================`
- Document every public method with `/** @param @return */` style comments
- Prefix private members with underscore: `_controllerNumber`, `_deviceConnected`
- Use `debugPrint()` for debug output. Raw `Serial.print` is only for always-on messages (connection events, errors, settings warnings)

## File Structure (Arduino Library Manager compliant)

```
src/DFPongController.h    # Public API, platform detection, class definition
src/DFPongController.cpp  # Implementation for both platforms
examples/Pong 2026/*/     # The 2026 starters, one folder per example with its .ino file
examples/Pong 2025/*/     # The earlier examples (00_StartTemplate to 09_KnobSteering)
library.properties        # Metadata (update version here)
keywords.txt              # IDE syntax highlighting
```

## Adding New Features

1. Add public method signature to `DFPongController.h` with documentation
2. Implement in `.cpp` with `#ifdef DFPONG_USE_NIMBLE` guards if BLE-related
3. Add to API Reference table in `README.md` and to `keywords.txt`
4. Update examples if the feature is commonly used
5. New game settings: add a named setter that clamps to the range in PROTOCOL.md and stores it with `storeNumber()` / `storeDecimal()` / `storeSetting()`

## Testing

Test page: https://digitalfuturesocadu.github.io/df-pong/game/cc26/?test=1 (2025 page: https://digitalfuturesocadu.github.io/df-pong/game/test/)
- Verify connection on all board types when modifying BLE code
- Compile every example for: `arduino:samd:nano_33_iot`, `esp32:esp32:esp32`, `esp32:esp32:esp32s3`, `esp32:esp32:esp32c3`, `arduino:esp32:nano_nora`, `arduino:mbed_nano:nano33ble`, `arduino:renesas_uno:unor4wifi`
- Check the 2025 game still works (it only knows the movement characteristic)

## Related Resources

- Game repo: https://github.com/DigitalFuturesOCADU/df-pong
- Protocol: https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md
- Controller examples (legacy): https://github.com/DigitalFuturesOCADU/df-pong/tree/main/controller
