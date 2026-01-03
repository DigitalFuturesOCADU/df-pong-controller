# GitHub Copilot Instructions

## Project Overview

Arduino library for BLE controllers that connect to the [DF Pong](https://github.com/DigitalFuturesOCADU/df-pong) browser game. Supports Arduino Nano 33 IoT, Nano 33 BLE, UNO R4 WiFi, and ESP32.

## Architecture

### Platform Abstraction Pattern
The library uses compile-time platform detection via `#ifdef DFPONG_USE_NIMBLE`:
- **ESP32**: Uses NimBLE-Arduino with callback classes (`ServerCallbacks`, `CharacteristicCallbacks`)
- **Arduino boards**: Uses ArduinoBLE with static event handlers (`onBLEConnected`, etc.)

All platform-specific code is isolated using `#ifdef`/`#else`/`#endif` blocks. When modifying BLE functionality, update BOTH implementations.

### Singleton Pattern
`DFPongController::_instance` provides static callback access. Only one controller instance is supported per device.

### UUID Generation
Controller numbers (1-242) generate unique BLE UUIDs via `generateUUIDs()`. UUIDs must match the JavaScript game exactly:
```cpp
// Base: "19b10010-e8f2-537e-4f6c-d104768a12" + hex(13 + controllerNumber)
// Controller 1 → suffix "0e" (14), Controller 2 → suffix "0f" (15)
```

### Connection Flow
1. `begin()` → Start advertising with generated UUIDs
2. Central connects → Send `HANDSHAKE` (value 3) until acknowledged
3. Central writes `HANDSHAKE` back → `_handshakeComplete = true`
4. `sendControl()` now sends actual direction values

## Beginner-Friendly API Guidelines

- **No pointers/references in public API** - Use simple `int`, `bool`, `const char*`
- **Method chaining setup**: `setControllerNumber()` → `setStatusLED()` → `begin()`
- **Mandatory loop call**: `update()` must be called every `loop()` iteration
- **Direction constants**: Always expose `UP`, `DOWN`, `NEUTRAL` as global constants

## Code Style

- Use descriptive section comments: `// ============================================`
- Document every public method with `/** @param @return */` style comments
- Prefix private members with underscore: `_controllerNumber`, `_deviceConnected`
- Use `debugPrint()` for debug output, never raw `Serial.print` in production paths

## File Structure (Arduino Library Manager compliant)

```
src/DFPongController.h    # Public API, platform detection, class definition
src/DFPongController.cpp  # Implementation for both platforms
examples/*/               # Each folder = one example with .ino file
library.properties        # Metadata (update version here)
keywords.txt              # IDE syntax highlighting
```

## Adding New Features

1. Add public method signature to `DFPongController.h` with documentation
2. Implement in `.cpp` with `#ifdef DFPONG_USE_NIMBLE` guards if BLE-related
3. Add to API Reference table in `README.md`
4. Update examples if the feature is commonly used

## Testing

Test page: https://digitalfuturesocadu.github.io/df-pong/game/test/
- Verify connection on all 4 board types when modifying BLE code
- ESP32 RSSI returns approximate value (-50) due to NimBLE limitations

## Related Resources

- Game repo: https://github.com/DigitalFuturesOCADU/df-pong
- Controller examples (legacy): https://github.com/DigitalFuturesOCADU/df-pong/tree/main/controller
