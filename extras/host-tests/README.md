# Host tests

Tests for the library's logic that run on a computer, with ArduinoBLE simulated (`mock/`): no board needed.
They cover the ArduinoBLE code path; the ESP32 (NimBLE) path is only compile-checked.

```bash
sh extras/host-tests/run.sh
```

- `eventtest.cpp`: game events (readGameEvent, onGameEvent, scores) and the buzzer (melodies, priorities,
  connected / dropped sounds).
