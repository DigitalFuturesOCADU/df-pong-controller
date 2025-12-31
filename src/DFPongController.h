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
 * - ESP32 (requires NimBLE-Arduino library)
 * 
 * Created by Digital Futures OCAD U
 * MIT License
 */

#ifndef DF_PONG_CONTROLLER_H
#define DF_PONG_CONTROLLER_H

#include <Arduino.h>

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
     * @param pin The LED pin (e.g., LED_BUILTIN)
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
    NimBLEAdvertising* _pAdvertising;
#else
    BLEService* _pongService;
    BLEByteCharacteristic* _movementCharacteristic;
#endif
    
    // UUID storage
    char _serviceUuid[37];
    char _characteristicUuid[37];
    
    // State tracking
    bool _serviceStarted;
    bool _handshakeComplete;
    bool _ledState;
    int _lastSentValue;
    bool _valueChanged;
    bool _deviceConnected;
    
    // Timing
    unsigned long _lastLedToggle;
    unsigned long _lastNotificationTime;
    unsigned long _connectionStartTime;
    
    // Constants
    static const unsigned long LED_BLINK_SLOW = 500;
    static const unsigned long LED_BLINK_FAST = 100;
    static const unsigned long MIN_NOTIFICATION_INTERVAL = 20;
    static const unsigned long HANDSHAKE_TIMEOUT = 5000;
    
    // Manufacturer data for device identification
    static const uint8_t MANUFACTURER_DATA[2];
    
    // Private methods
    void generateUUIDs();
    void updateLED();
    void resetState();
    void debugPrint(const char* message);
    void debugPrint(const char* message, int value);
    
    // Event handlers - platform specific
    static DFPongController* _instance;
    
#ifdef DFPONG_USE_NIMBLE
    // NimBLE callback classes
    class ServerCallbacks;
    class CharacteristicCallbacks;
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
#else
    static void onBLEConnected(BLEDevice central);
    static void onBLEDisconnected(BLEDevice central);
    static void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);
#endif
};

#endif // DF_PONG_CONTROLLER_H
