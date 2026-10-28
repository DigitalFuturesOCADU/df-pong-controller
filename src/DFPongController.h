/*
 * DFPongController.h
 * 
 * A beginner-friendly BLE controller library for the DF Pong game.
 * https://github.com/DigitalFuturesOCADU/df-pong
 * 
 * Supported Boards:
 * - Arduino Nano 33 IoT
 * - Arduino Nano 33 BLE / BLE Sense
 * - Arduino UNO R4 WiFi
 * - ESP32 / ESP32-S3 / ESP32-C3 / Arduino Nano ESP32
 *   (requires the NimBLE-Arduino library, version 2.x)
 * 
 * Protocol reference:
 * https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md
 * 
 * Created by Digital Futures OCAD U
 * MIT License
 */

#ifndef DF_PONG_CONTROLLER_H
#define DF_PONG_CONTROLLER_H

#include <Arduino.h>
#include "DFPongSpec.h"  // the DFPONG API: ranges, names and limits shared with the game (generated)

// ============================================
// Platform Detection
// ============================================
#if defined(ESP32)
    #define DFPONG_USE_NIMBLE
    #include <NimBLEDevice.h>
#else
    #define DFPONG_USE_ARDUINOBLE
    #include <ArduinoBLE.h>
#endif

// ============================================
// LED_BUILTIN fallback (some ESP32 boards)
// ============================================
// Some ESP32 board definitions (for example "ESP32 Dev Module") don't define
// LED_BUILTIN, so sketches that use it would not compile. On those boards
// LED_BUILTIN becomes -1, which means "no status LED". (We don't guess a pin:
// GPIO 2 is a common LED pin, but the examples use pin 2 for a button.)
// If your board has an LED, use its pin number: controller.setStatusLED(2);
// The Arduino Nano ESP32 defines LED_BUILTIN in a way the preprocessor
// can't see, so it is skipped here on purpose.
#if defined(ESP32) && !defined(LED_BUILTIN) && !defined(ARDUINO_NANO_ESP32)
    #define LED_BUILTIN -1
#endif

// ============================================
// Library Version
// ============================================
#define DFPONG_CONTROLLER_VERSION "2.4.0"

// ============================================
// Direction Constants
// Use these with sendControl()
// ============================================
const int NEUTRAL = 0;    // No movement / idle
const int UP = 1;         // Paddle moves up
const int DOWN = 2;       // Paddle moves down

// ============================================
// Internal Constants (do not modify)
// ============================================
const int HANDSHAKE = 3;  // Connection handshake signal

// ============================================
// DFPongController Class
// ============================================
class DFPongController {
public:
    // ----------------------------------------
    // Constructor
    // ----------------------------------------
    DFPongController();
    
    // ----------------------------------------
    // Configuration (call before begin())
    // ----------------------------------------
    
    /**
     * Set your unique controller number.
     * REQUIRED: Call this before begin()!
     * 
     * @param number Your assigned number (1-242)
     *               Each player needs a unique number.
     */
    void setControllerNumber(int number);
    
    /**
     * Set the pin for connection status LED.
     * Optional: LED will blink when disconnected, solid when ready.
     * 
     * @param pin The LED pin (e.g., LED_BUILTIN). -1 = no LED.
     */
    void setStatusLED(int pin);
    
    /**
     * Enable/disable debug messages to Serial.
     * 
     * @param enabled true to print debug info
     */
    void setDebug(bool enabled);
    
    // ----------------------------------------
    // Initialization
    // ----------------------------------------
    
    /**
     * Initialize the BLE controller.
     * Uses default name "DFPONG-X" where X is controller number.
     * 
     * @return true if successful, false if failed
     */
    bool begin();
    
    /**
     * Initialize the BLE controller with custom name.
     * 
     * @param deviceName Custom Bluetooth device name
     * @return true if successful, false if failed
     */
    bool begin(const char* deviceName);
    
    // ----------------------------------------
    // Main Loop (REQUIRED in loop())
    // ----------------------------------------
    
    /**
     * Update BLE connection and internal state.
     * MUST be called every loop() iteration!
     * 
     * It also takes care of the connection handshake and sends
     * any settings you changed, so you don't have to.
     */
    void update();
    
    // ----------------------------------------
    // Sending Controls
    // ----------------------------------------
    
    /**
     * Send a control direction to the game.
     * 
     * @param direction UP, DOWN, or NEUTRAL
     */
    void sendControl(int direction);
    
    // ----------------------------------------
    // Actions (experimental: the 2026 game, DFPONG API 1.1)
    // ----------------------------------------
    // Things that happen during play. Older games ignore them, so they never
    // stop a controller from working there.
    
    /**
     * Steer the paddle to a height instead of using UP / DOWN: for knobs,
     * sliders and distance sensors. The paddle still moves at its normal
     * speed, so this is smoother than buttons, not faster. Call it as often
     * as you like: only changes are sent (at most every 20 ms).
     * 
     * @param position 0.0 (top) to 1.0 (bottom)
     */
    void sendPosition(float position);
    
    /**
     * Steer from a sensor reading. top is the reading that puts the paddle at
     * the top, bottom the reading that puts it at the bottom. Any numbers work:
     *     controller.sendPosition(analogRead(A0), 0, 1023);   // knob
     *     controller.sendPosition(distanceCm, 5, 40);         // 5 cm = top, 40 cm = bottom
     * Swap top and bottom to turn the direction around. Readings past either
     * end keep the paddle at that end.
     */
    void sendPosition(long reading, long top, long bottom);
    
    /**
     * Go back to UP / DOWN control. Sending UP or DOWN with sendControl()
     * does this too (NEUTRAL doesn't).
     */
    void stopPosition();
    
    /**
     * Taunt: a word pops up beside your paddle, with a sound. The game shows
     * at most one every 2 seconds per player, and the host can turn taunts off.
     * 
     * @param emote 0 "HA!", 1 "NICE", 2 "GG", 3 "OOPS", 4 "COME ON", 5 "WOW"
     */
    void sendTaunt(int emote = 0);
    
    /**
     * True when the game is listening for actions (the 2026 game).
     */
    bool actionsAvailable();
    
    // ----------------------------------------
    // Connection Status
    // ----------------------------------------
    
    /**
     * Check if BLE is connected to the game.
     * 
     * @return true if Bluetooth connected
     */
    bool isConnected();
    
    /**
     * Check if controller is fully ready to play.
     * (Connected AND handshake complete)
     * 
     * @return true if ready to send controls
     */
    bool isReady();
    
    // ----------------------------------------
    // Signal Strength
    // ----------------------------------------
    
    /**
     * Get the current signal strength (RSSI).
     * Useful for debugging connection issues.
     * 
     * @return RSSI in dBm (e.g., -50 = excellent, -90 = poor)
     *         Returns 0 if not connected.
     */
    int getRSSI();
    
    /**
     * Quick check if signal strength is good.
     * 
     * @return true if RSSI > threshold (default -70 dBm)
     */
    bool hasStrongSignal();
    
    /**
     * Set the threshold for hasStrongSignal().
     * 
     * @param dBm The threshold in dBm (e.g., -70)
     */
    void setRSSIThreshold(int dBm);
    
    // ----------------------------------------
    // Information
    // ----------------------------------------
    
    /**
     * Get the configured controller number.
     * 
     * @return The controller number (1-242)
     */
    int getControllerNumber();
    
    /**
     * Get the generated service UUID.
     * Useful for debugging.
     * 
     * @return The BLE service UUID string
     */
    const char* getServiceUUID();
    
    // ----------------------------------------
    // Player Settings (new in 2.0)
    // ----------------------------------------
    // Customize how YOUR paddle looks, moves and sounds in the game.
    // You can call these before OR after begin(). If you change a
    // setting while connected, the game picks it up automatically.
    // Values that are out of range are fixed for you, and a warning
    // is printed to the Serial Monitor.
    
    /**
     * Set your controller's name in the game, e.g. setControllerName("Thunderpaddle").
     * The name you set here comes first. Without it, the game uses the
     * controller name from the class list (players-config.json), then your
     * own name from the class list.
     *
     * @param name Up to 20 characters (longer names are shortened).
     *             Use "" to go back to the name from the class list.
     */
    void setControllerName(const char* name);
    void setControllerName(const String& name);
    
    /** Same as setControllerName() (its name before library 2.4). */
    void setPlayerName(const char* name);
    void setPlayerName(const String& name);
    
    /**
     * Set your paddle color using red, green and blue amounts.
     * 
     * @param r Red   (0-255)
     * @param g Green (0-255)
     * @param b Blue  (0-255)
     */
    void setPlayerColor(int r, int g, int b);
    
    /**
     * Set your paddle color using a color code.
     * 
     * @param cssColor A web color like "#ff8800" (any CSS color works,
     *                 e.g. "orange"). Use "" to go back to the default.
     */
    void setPlayerColor(const char* cssColor);
    void setPlayerColor(const String& cssColor);
    
    /**
     * Set how fast your paddle moves.
     * 
     * @param speed 1 (very slow) to 100 (very fast). The game's normal speed is 10.
     */
    void setMoveSpeed(int speed);
    
    /**
     * Set the size of your paddle.
     * 
     * @param percent 50 (half size) to 200 (double size). Normal is 100.
     */
    void setPaddleSize(int percent);
    
    /**
     * Set the sound played when YOUR paddle hits the puck.
     * 
     * @param nameOrUrl A built-in sound name (e.g. "pingpong", "claves", "bell",
     *                  "gong", "bikebell", "pluck", "blip", "laser", "coin";
     *                  full list in the README) or an https:// link to a
     *                  short sound file (200 characters max).
     *                  Use "" to go back to the default sound.
     */
    void setHitSound(const char* nameOrUrl);
    void setHitSound(const String& nameOrUrl);
    
    /**
     * Set the sound played when YOU score a point.
     * 
     * @param nameOrUrl A built-in sound name or an https:// link (see setHitSound).
     */
    void setScoreSound(const char* nameOrUrl);
    void setScoreSound(const String& nameOrUrl);
    
    /**
     * Set the sound played when YOU win the match.
     * 
     * @param nameOrUrl A built-in sound name or an https:// link (see setHitSound).
     */
    void setWinSound(const char* nameOrUrl);
    void setWinSound(const String& nameOrUrl);
    
    /**
     * Set the background music that plays during the match (new in 2.1).
     * If your opponent brings music too, the game swaps between your picks
     * each point, or whoever is at the computer can choose one.
     * 
     * @param nameOrUrl A music track name (e.g. "nightbus", "gymnopedie";
     *                  full list in the README) or an https:// link
     *                  to a music file (200 characters max).
     *                  Use "" to go back to the room's music.
     */
    void setMusic(const char* nameOrUrl);
    void setMusic(const String& nameOrUrl);
    
    // ----------------------------------------
    // Game Setting Requests (new in 2.0)
    // ----------------------------------------
    // These settings are shared by both players, so they are REQUESTS.
    // When the match starts, the game combines both players' requests
    // (for example by averaging them) and shows the result on screen.
    
    /**
     * Ask for the number of points needed to win.
     * 
     * @param points 1-21 (the game's default is 10)
     */
    void requestPointsToWin(int points);
    
    /**
     * Ask for the starting speed of the puck.
     * 
     * @param speed 1-20 (the game's default is 5)
     */
    void requestPuckSpeed(float speed);
    
    /**
     * Ask for how much faster the puck gets after each hit.
     * 
     * @param increment 0 (never speeds up) to 1 (the game's default is 0.15)
     */
    void requestSpeedIncrement(float increment);
    
    // ----------------------------------------
    // Any Setting (advanced, new in 2.0)
    // ----------------------------------------
    // Use these for settings the game adds in the future, before this
    // library has a special function for them. The key must match the
    // game's setting name exactly (letters, numbers, _ and - only).
    // These do NOT check ranges - the game will do that.
    
    /**
     * Set any setting by its key name.
     * Examples:
     *   controller.setSetting("trailLength", 5);
     *   controller.setSetting("glow", 0.5);
     *   controller.setSetting("rainbow", true);
     *   controller.setSetting("emoji", "rocket");
     * 
     * @param key   The setting name, e.g. "moveSpeed"
     * @param value A whole number, decimal number, true/false, or text
     */
    void setSetting(const char* key, int value);
    void setSetting(const char* key, unsigned int value);
    void setSetting(const char* key, long value);
    void setSetting(const char* key, unsigned long value);
    void setSetting(const char* key, double value);
    void setSetting(const char* key, bool value);
    void setSetting(const char* key, const char* value);
    void setSetting(const char* key, const String& value);
    void setSetting(const char* key, const __FlashStringHelper* value);
    
    /**
     * Remove one setting, so the game uses its default again.
     * 
     * @param key The setting name, e.g. "moveSpeed"
     */
    void clearSetting(const char* key);
    
    /**
     * Remove ALL of your settings, so the game uses its defaults.
     */
    void clearSettings();
    
    /**
     * Get the settings exactly as the game will read them (JSON text).
     * Handy for checking: Serial.println(controller.getSettingsJSON());
     * 
     * @return The settings JSON, e.g. {"v":1,"name":"Kate","moveSpeed":14}
     */
    const char* getSettingsJSON();

private:
    // Configuration
    int _controllerNumber;
    int _statusLedPin;
    bool _debug;
    int _rssiThreshold;
    
    // BLE objects - platform specific
#ifdef DFPONG_USE_NIMBLE
    NimBLEServer* _pServer;
    NimBLEService* _pongService;
    NimBLECharacteristic* _movementCharacteristic;
    NimBLECharacteristic* _configCharacteristic;
    NimBLECharacteristic* _actionsCharacteristic;
    NimBLEAdvertising* _pAdvertising;
#else
    BLEService* _pongService;
    BLEByteCharacteristic* _movementCharacteristic;
    BLECharacteristic* _configCharacteristic;
    BLECharacteristic* _actionsCharacteristic;
#endif

    // UUID storage
    char _serviceUuid[37];
    char _characteristicUuid[37];
    char _configUuid[37];
    char _actionsUuid[37];
    
    // State written by the BLE event handlers.
    // On ESP32 the handlers run in a separate task, hence "volatile".
    volatile bool _deviceConnected;
    volatile bool _subscribed;            // NimBLE only (ArduinoBLE asks the characteristic)
    volatile bool _actionsSubscribed;     // NimBLE only, for the actions characteristic
    volatile bool _handshakeComplete;
    volatile uint16_t _connHandle;        // NimBLE only
    volatile uint8_t _connectionEvents;   // Changes on every connect / disconnect
    volatile unsigned long _connectionStartTime;
    
    // State used only by update() / sendControl()
    bool _serviceStarted;
    bool _ledState;
    int _lastSentValue;                   // Last movement value notified (-1 = none yet)
    int _desiredValue;                    // Movement value the game should have
    bool _controlCalledThisLoop;          // Track if sendControl was called this loop
    bool _controlCalledPreviousLoop;      // Track if sendControl was called last loop
    uint8_t _seenConnectionEvents;
    bool _wasSubscribed;
    bool _disconnectRequested;
    uint16_t _handshakesSent;
    int _desiredPosition;                 // 0-255 the sketch asked for (-1 = steering with UP / DOWN)
    int _lastSentPosition;                // Last position sent on this connection (-1 = none)
    unsigned long _lastPositionTime;
    unsigned long _lastTauntTime;
    
    // Timing
    unsigned long _lastLedToggle;
    unsigned long _lastNotificationTime;
    unsigned long _subscribeTime;
    unsigned long _lastHandshakeTime;
    unsigned long _lastSettingsPush;
    
    // Settings (config characteristic).
    // The JSON text is edited in place: {"v":1,"key":value,...}
    static const uint16_t SETTINGS_MAX_LENGTH = DFPongSpec::CONFIG_MAX_BYTES;
    char _settingsJson[SETTINGS_MAX_LENGTH + 1];
    uint16_t _settingsLength;
    bool _settingsDirty;
    
    // Constants
    static const unsigned long LED_BLINK_SLOW = 500;
    static const unsigned long LED_BLINK_FAST = 100;
    static const unsigned long MIN_NOTIFICATION_INTERVAL = 20;
    static const unsigned long HANDSHAKE_INTERVAL = DFPongSpec::HANDSHAKE_REPEAT_MS;
    static const unsigned long HANDSHAKE_TIMEOUT = DFPongSpec::HANDSHAKE_TIMEOUT_MS;  // counted from subscribing
    static const unsigned long SUBSCRIBE_TIMEOUT = DFPongSpec::SUBSCRIBE_TIMEOUT_MS;  // counted from connecting
    static const unsigned long SETTINGS_UPDATE_INTERVAL = DFPongSpec::CONFIG_MIN_UPDATE_MS;
    
    // Manufacturer data for device identification
    static const uint8_t MANUFACTURER_DATA[2];
    
    // Private methods
    void generateUUIDs();
    void updateLED();
    void resetState();
    void handleHandshake(unsigned long now);
    void flushMovement(unsigned long now);
    bool notifyMovement(uint8_t value);
    bool isSubscribed();
    void disconnectCentral();
    void pushSettings(unsigned long now);
    void writeConfigValue();
    void storeName(const char* caller, const char* name);
    void flushPosition(unsigned long now);
    bool notifyAction(const uint8_t* data, uint8_t length);
    void debugPrint(const char* message);
    void debugPrint(const char* message, int value);
    
    // Settings helpers
    bool storeSetting(const char* key, const char* value, bool isText);
    bool storeNumber(const char* key, long value);
    bool storeDecimal(const char* key, double value, const char* caller);
    bool removeSetting(const char* key);
    bool findSetting(const char* key, uint16_t& start, uint16_t& end);
    bool isValidKey(const char* key, const char* caller);
    void setSound(const char* key, const char* caller, const char* nameOrUrl);
    void setMedia(const char* key, const char* caller, const char* nameOrUrl,
                  const char* const* names, size_t nameCount, const char* kind);
    long clampInt(const char* caller, const char* part, long value, long minValue, long maxValue);
    bool clampDecimal(const char* caller, double& value, double minValue, double maxValue);
    
    // Event handlers - platform specific
    static DFPongController* _instance;

#ifdef DFPONG_USE_NIMBLE
    // NimBLE callback classes
    class ServerCallbacks;
    class CharacteristicCallbacks;
    class ActionsCallbacks;
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
    friend class ActionsCallbacks;
#else
    static void onBLEConnected(BLEDevice central);
    static void onBLEDisconnected(BLEDevice central);
    static void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);
#endif
};

#endif // DF_PONG_CONTROLLER_H
