/*
 * DFPongController.cpp
 * 
 * Implementation of the DFPongController library.
 * Supports both ArduinoBLE (Arduino boards) and NimBLE (ESP32).
 * 
 * Protocol reference:
 * https://github.com/DigitalFuturesOCADU/df-pong/blob/main/docs/PROTOCOL.md
 * 
 * Created by Digital Futures OCAD U
 * MIT License
 */

#include "DFPongController.h"
#include <string.h>
#include <ctype.h>

// ============================================
// Static Members
// ============================================

// Singleton instance pointer for callbacks
DFPongController* DFPongController::_instance = nullptr;

// Manufacturer data: 0xDF = DFPong, 0x01 = version 1
const uint8_t DFPongController::MANUFACTURER_DATA[2] = {0xDF, 0x01};

// ============================================
// Settings Helpers (internal)
// ============================================

namespace {

// The settings JSON always starts with the schema version.
const char SETTINGS_EMPTY[] = "{\"v\":1}";
const uint16_t SETTINGS_EMPTY_LENGTH = 7;      // strlen(SETTINGS_EMPTY)
const uint16_t SETTINGS_HEADER_LENGTH = 6;     // strlen("{\"v\":1")

const size_t MAX_KEY_LENGTH = 32;
const size_t MAX_NAME_CHARACTERS = 20;
const size_t MAX_SOUND_LENGTH = 200;

const uint16_t NO_CONNECTION = 0xFFFF;

const char HEX_DIGITS[] = "0123456789abcdef";

// Sounds the game can always play (they are synthesized in the browser).
const char* const BUILT_IN_SOUNDS[] = {
    // recorded
    "pingpong", "claves", "tick", "bell", "gong", "bikebell", "flourish",
    "applause", "pip", "pipgo", "quindar", "clock",
    // synthesized
    "blip", "boop", "pluck", "click", "laser", "drum", "chirp", "zap",
    "thud", "wood", "glass", "coin",
    // the 2024 originals
    "classic", "wall", "ding"
};
const size_t BUILT_IN_SOUND_COUNT = sizeof(BUILT_IN_SOUNDS) / sizeof(BUILT_IN_SOUNDS[0]);

const char* const BUILT_IN_MUSIC[] = { "drift", "tabletop", "arcade", "nightbus" };
const size_t BUILT_IN_MUSIC_COUNT = sizeof(BUILT_IN_MUSIC) / sizeof(BUILT_IN_MUSIC[0]);

// The game ignores upper/lower case in names
bool equalsIgnoreCase(const char* a, const char* b) {
    for (; *a && *b; a++, b++) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    }
    return *a == *b;
}

enum EmitMode { EMIT_COUNT, EMIT_COMPARE, EMIT_WRITE };

// Produces one settings entry:  ,"key":value
// - EMIT_COUNT   only returns the length
// - EMIT_COMPARE checks whether dest already holds exactly this entry
// - EMIT_WRITE   writes the entry to dest
// Text values are quoted and escaped. Keys are checked by isValidKey()
// before they get here, so they never need escaping.
size_t emitEntry(char* dest, EmitMode mode, bool* same,
                 const char* key, const char* value, bool isText) {
    size_t n = 0;
    auto put = [&](char c) {
        if (mode == EMIT_WRITE) {
            dest[n] = c;
        } else if (mode == EMIT_COMPARE && dest[n] != c) {
            *same = false;
        }
        n++;
    };
    
    put(',');
    put('"');
    for (const char* k = key; *k; k++) put(*k);
    put('"');
    put(':');
    
    if (!isText) {
        for (const char* v = value; *v; v++) put(*v);
        return n;
    }
    
    put('"');
    for (const unsigned char* v = (const unsigned char*)value; *v; v++) {
        unsigned char c = *v;
        if (c == '"' || c == '\\') {
            put('\\');
            put((char)c);
        } else if (c == '\n') {
            put('\\');
            put('n');
        } else if (c == '\r') {
            put('\\');
            put('r');
        } else if (c == '\t') {
            put('\\');
            put('t');
        } else if (c == '\b') {
            put('\\');
            put('b');
        } else if (c == '\f') {
            put('\\');
            put('f');
        } else if (c < 0x20) {
            put('\\');
            put('u');
            put('0');
            put('0');
            put(HEX_DIGITS[c >> 4]);
            put(HEX_DIGITS[c & 0x0F]);
        } else {
            put((char)c);   // includes UTF-8 bytes, which JSON allows as-is
        }
    }
    put('"');
    return n;
}

// Writes a decimal number with up to 4 decimal places ("0.15", "7", "-2.5").
// Doesn't use printf("%f"), which is missing or large on some boards.
// Returns false for values JSON can't hold (NaN, infinity) or huge values.
bool formatDecimal(double value, char* out, size_t outSize) {
    if (outSize < 22) return false;
    if (!(value > -1e14 && value < 1e14)) return false;   // also catches NaN
    
    bool negative = value < 0;
    if (negative) value = -value;
    
    unsigned long long scaled = (unsigned long long)(value * 10000.0 + 0.5);
    if (scaled == 0) negative = false;   // no "-0"
    
    unsigned long long whole = scaled / 10000ULL;
    unsigned int fraction = (unsigned int)(scaled % 10000ULL);
    
    char digits[20];
    int count = 0;
    do {
        digits[count++] = (char)('0' + (int)(whole % 10ULL));
        whole /= 10ULL;
    } while (whole > 0 && count < 20);
    
    size_t pos = 0;
    if (negative) out[pos++] = '-';
    while (count > 0) out[pos++] = digits[--count];
    
    if (fraction > 0) {
        char f[4];
        for (int i = 3; i >= 0; i--) {
            f[i] = (char)('0' + (fraction % 10));
            fraction /= 10;
        }
        int last = 3;
        while (last > 0 && f[last] == '0') last--;
        out[pos++] = '.';
        for (int i = 0; i <= last; i++) out[pos++] = f[i];
    }
    out[pos] = '\0';
    return true;
}

bool startsWithIgnoreCase(const char* text, const char* prefix) {
    while (*prefix) {
        if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix)) return false;
        text++;
        prefix++;
    }
    return true;
}

bool isHexText(const char* text) {
    if (*text == '\0') return false;
    for (; *text; text++) {
        if (!isxdigit((unsigned char)*text)) return false;
    }
    return true;
}

void printWarningStart(const char* caller) {
    Serial.print("DFPong: ");
    Serial.print(caller);
}

void printDecimal(double value) {
    char text[24];
    if (formatDecimal(value, text, sizeof(text))) {
        Serial.print(text);
    } else {
        Serial.print(value);
    }
}

} // namespace

// ArduinoBLE 2.1.0 answers a read that starts exactly at the end of a value
// with an "Invalid Offset" error instead of an empty reply. Browsers read long
// values in packet-sized pieces, so a settings string whose length is an exact
// multiple of the packet size could fail to load. These are the packet sizes
// (MTU - 1) seen with common boards, computers and phones.
static bool isPacketBoundary(size_t length) {
    static const uint16_t packetSizes[] = {22, 184, 241, 243, 246, 250, 511};
    for (size_t i = 0; i < sizeof(packetSizes) / sizeof(packetSizes[0]); i++) {
        if (length % packetSizes[i] == 0) return true;
    }
    return false;
}

// ============================================
// NimBLE Callback Classes (ESP32 only)
// ============================================
// These run inside the NimBLE task, not inside loop(). They only update
// a few flags; update() does the rest from loop().

#ifdef DFPONG_USE_NIMBLE

class DFPongController::ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*, NimBLEConnInfo& connInfo) override {
        DFPongController* controller = DFPongController::_instance;
        if (controller == nullptr) return;
        
        controller->_connectionStartTime = millis();
        controller->_handshakeComplete = false;
        controller->_subscribed = false;
        controller->_connHandle = connInfo.getConnHandle();
        controller->_deviceConnected = true;
        controller->_connectionEvents = (uint8_t)(controller->_connectionEvents + 1);
        
        Serial.print("Connected to: ");
        Serial.println(connInfo.getAddress().toString().c_str());
    }
    
    void onDisconnect(NimBLEServer*, NimBLEConnInfo& connInfo, int) override {
        DFPongController* controller = DFPongController::_instance;
        if (controller == nullptr) return;
        
        controller->_deviceConnected = false;
        controller->_subscribed = false;
        controller->_handshakeComplete = false;
        controller->_connHandle = NO_CONNECTION;
        controller->_connectionEvents = (uint8_t)(controller->_connectionEvents + 1);
        
        Serial.print("Disconnected from: ");
        Serial.println(connInfo.getAddress().toString().c_str());
        Serial.println("Waiting for connection...");
        
        // Advertising restarts automatically (see advertiseOnDisconnect in begin())
    }
};

class DFPongController::CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo&) override {
        DFPongController* controller = DFPongController::_instance;
        if (controller == nullptr) return;
        
        // Ignore empty writes
        if (pCharacteristic->getLength() < 1) return;
        
        uint8_t value = pCharacteristic->getValue<uint8_t>();
        
        if (value == HANDSHAKE && !controller->_handshakeComplete) {
            controller->_handshakeComplete = true;
            controller->debugPrint("Handshake complete!");
            Serial.println("Controller ready to play!");
        }
    }
    
    void onSubscribe(NimBLECharacteristic*, NimBLEConnInfo&, uint16_t subValue) override {
        DFPongController* controller = DFPongController::_instance;
        if (controller == nullptr) return;
        
        // subValue: 0 = unsubscribed, 1 = notifications, 2 = indications, 3 = both
        controller->_subscribed = (subValue != 0);
    }
};

#endif // DFPONG_USE_NIMBLE

// ============================================
// Constructor
// ============================================

DFPongController::DFPongController() {
    _controllerNumber = 0;  // Invalid until set
    _statusLedPin = -1;     // No LED until set
    _debug = false;
    _rssiThreshold = -70;   // Default: -70 dBm

#ifdef DFPONG_USE_NIMBLE
    _pServer = nullptr;
    _pAdvertising = nullptr;
#endif
    _pongService = nullptr;
    _movementCharacteristic = nullptr;
    _configCharacteristic = nullptr;
    
    _serviceUuid[0] = '\0';
    _characteristicUuid[0] = '\0';
    _configUuid[0] = '\0';
    
    _deviceConnected = false;
    _subscribed = false;
    _handshakeComplete = false;
    _connHandle = NO_CONNECTION;
    _connectionEvents = 0;
    _connectionStartTime = 0;
    
    _serviceStarted = false;
    _ledState = false;
    _seenConnectionEvents = 0;
    _lastLedToggle = 0;
    resetState();
    
    // Start with no custom settings
    memcpy(_settingsJson, SETTINGS_EMPTY, SETTINGS_EMPTY_LENGTH + 1);
    _settingsLength = SETTINGS_EMPTY_LENGTH;
    _settingsDirty = false;
    _lastSettingsPush = 0;
    
    // Set singleton instance
    _instance = this;
}

// ============================================
// Configuration Methods
// ============================================

void DFPongController::setControllerNumber(int number) {
    // The number is part of the Bluetooth UUIDs, which can't change after begin()
    if (_serviceStarted) {
        if (number != _controllerNumber) {
            Serial.print("DFPong: setControllerNumber() must be called before begin(). Still using #");
            Serial.println(_controllerNumber);
        }
        return;
    }
    
    _controllerNumber = number;
    if (number >= 1 && number <= 242) {
        generateUUIDs();
    }
    
    if (_debug) {
        Serial.print("Controller number set to: ");
        Serial.println(number);
    }
}

void DFPongController::setStatusLED(int pin) {
    if (pin < 0) {
        // No LED (e.g. LED_BUILTIN on an ESP32 board that doesn't have one)
        _statusLedPin = -1;
        Serial.println("DFPong: no status LED. If your board has an LED, use its pin number,");
        Serial.println("        for example: controller.setStatusLED(2);");
        return;
    }
    _statusLedPin = pin;
    pinMode(_statusLedPin, OUTPUT);
    digitalWrite(_statusLedPin, LOW);
}

void DFPongController::setDebug(bool enabled) {
    _debug = enabled;
}

void DFPongController::setRSSIThreshold(int dBm) {
    _rssiThreshold = dBm;
}

// ============================================
// UUID Generation
// ============================================

void DFPongController::generateUUIDs() {
    // Base UUIDs (must match JavaScript exactly)
    const char* serviceBase = "19b10010-e8f2-537e-4f6c-d104768a12";
    const char* characteristicBase = "19b10011-e8f2-537e-4f6c-d104768a12";
    const char* configBase = "19b10012-e8f2-537e-4f6c-d104768a12";
    
    // Calculate unique suffix: device 1 -> 14 (0x0e), device 2 -> 15 (0x0f), etc.
    unsigned int suffix = (unsigned int)(13 + _controllerNumber) & 0xFF;
    
    // Generate full UUIDs with 2-digit hex suffix
    snprintf(_serviceUuid, sizeof(_serviceUuid), "%s%02x", serviceBase, suffix);
    snprintf(_characteristicUuid, sizeof(_characteristicUuid), "%s%02x", characteristicBase, suffix);
    snprintf(_configUuid, sizeof(_configUuid), "%s%02x", configBase, suffix);
}

// ============================================
// Initialization
// ============================================

bool DFPongController::begin() {
    // Generate default device name
    char defaultName[20];
    snprintf(defaultName, sizeof(defaultName), "DFPONG-%d", _controllerNumber);
    return begin(defaultName);
}

bool DFPongController::begin(const char* deviceName) {
    // Calling begin() twice would create a second copy of the service
    if (_serviceStarted) {
        Serial.println("DFPong: begin() was already called - ignoring the extra call.");
        return true;
    }
    
    if (deviceName == nullptr || deviceName[0] == '\0') {
        return begin();
    }
    
    // Validate controller number
    if (_controllerNumber < 1 || _controllerNumber > 242) {
        Serial.println("========================================");
        Serial.println("ERROR: Call setControllerNumber(1-242)");
        Serial.println("       before calling begin()!");
        Serial.println("========================================");
        return false;
    }
    
    debugPrint("Initializing DFPongController...");
    debugPrint("Controller #", _controllerNumber);
    
    // Generate unique UUIDs based on controller number
    generateUUIDs();
    if (_debug) {
        Serial.print("Service UUID: ");
        Serial.println(_serviceUuid);
        Serial.print("Characteristic UUID: ");
        Serial.println(_characteristicUuid);
        Serial.print("Config UUID: ");
        Serial.println(_configUuid);
    }

#ifdef DFPONG_USE_NIMBLE
    // ========== NimBLE (ESP32) Implementation ==========
    
    debugPrint("Starting NimBLE...");
    
    // Initialize NimBLE (waits until the Bluetooth stack is ready)
    if (!NimBLEDevice::init(deviceName)) {
        Serial.println("ERROR: BLE failed to initialize!");
        return false;
    }
    
    // Set power level for better range (+9 dBm)
    NimBLEDevice::setPower(9);
    
    // Create the server, service and characteristics only once,
    // even if begin() is retried after a failure
    if (_pServer == nullptr) {
        _pServer = NimBLEDevice::createServer();
        _pServer->setCallbacks(new ServerCallbacks());
        _pServer->advertiseOnDisconnect(true);
        
        // Create service
        _pongService = _pServer->createService(_serviceUuid);
        
        // Movement characteristic: read, write, notify (1 byte)
        _movementCharacteristic = _pongService->createCharacteristic(
            _characteristicUuid,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
        );
        _movementCharacteristic->setCallbacks(new CharacteristicCallbacks());
        uint8_t initialValue = NEUTRAL;
        _movementCharacteristic->setValue(&initialValue, 1);
        
        // Config characteristic: read, notify (settings JSON, up to 512 bytes)
        _configCharacteristic = _pongService->createCharacteristic(
            _configUuid,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY,
            SETTINGS_MAX_LENGTH
        );
        
        // Configure advertising: the service UUID lets the browser find us,
        // the name (in the scan response) shows up in the browser's device list
        _pAdvertising = NimBLEDevice::getAdvertising();
        _pAdvertising->addServiceUUID(_serviceUuid);
        _pAdvertising->enableScanResponse(true);
        _pAdvertising->setName(deviceName);
    }
    
    // Current settings
    writeConfigValue();
    
    // Start advertising (this also starts the GATT server)
    if (!NimBLEDevice::startAdvertising()) {
        Serial.println("ERROR: BLE advertising failed to start!");
        return false;
    }

#else
    // ========== ArduinoBLE Implementation ==========
    
    // Create BLE service and characteristics only once,
    // even if begin() is retried after a failure
    if (_pongService == nullptr) {
        _pongService = new BLEService(_serviceUuid);
        _movementCharacteristic = new BLEByteCharacteristic(
            _characteristicUuid,
            BLERead | BLENotify | BLEWrite
        );
        _configCharacteristic = new BLECharacteristic(
            _configUuid,
            BLERead | BLENotify,
            SETTINGS_MAX_LENGTH
        );
    }
    
    // Initialize BLE with retry
    debugPrint("Starting BLE...");
    bool bleStarted = false;
    for (int i = 0; i < 3; i++) {
        if (BLE.begin()) {
            bleStarted = true;
            break;
        }
        debugPrint("BLE init retry", i + 1);
        delay(500);
    }
    
    if (!bleStarted) {
        Serial.println("ERROR: BLE failed to initialize!");
        return false;
    }
    
    // Reset BLE state for clean start
    BLE.disconnect();
    delay(100);
    BLE.stopAdvertise();
    delay(100);
    
    // Configure event handlers
    BLE.setEventHandler(BLEConnected, onBLEConnected);
    BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
    _movementCharacteristic->setEventHandler(BLEWritten, onCharacteristicWritten);
    
    // Configure BLE parameters
    BLE.setLocalName(deviceName);
    BLE.setAdvertisedServiceUuid(_pongService->uuid());
    
    // Optimized connection parameters for crowded environments
    BLE.setConnectionInterval(12, 24);   // 15-30ms
    BLE.setPairable(false);
    BLE.setAdvertisingInterval(160);     // 100ms
    
    // Add manufacturer data for device identification
    BLE.setManufacturerData(MANUFACTURER_DATA, sizeof(MANUFACTURER_DATA));
    
    // Add characteristics to service and service to BLE
    _pongService->addCharacteristic(*_movementCharacteristic);
    _pongService->addCharacteristic(*_configCharacteristic);
    BLE.addService(*_pongService);
    
    // Set initial values
    _movementCharacteristic->writeValue((byte)NEUTRAL);
    writeConfigValue();
    delay(100);
    
    // Start advertising
    if (!BLE.advertise()) {
        Serial.println("WARNING: BLE.advertise() reported an error.");
    }

#endif // DFPONG_USE_NIMBLE

    _serviceStarted = true;
    _settingsDirty = false;
    _lastSettingsPush = millis();
    _seenConnectionEvents = _connectionEvents;
    resetState();
    
    Serial.println("========================================");
    Serial.print("DF Pong Controller #");
    Serial.print(_controllerNumber);
    Serial.println(" Ready!");
    Serial.print("Device Name: ");
    Serial.println(deviceName);
#ifdef DFPONG_USE_NIMBLE
    Serial.println("Platform: ESP32 (NimBLE)");
#else
    Serial.println("Platform: Arduino (ArduinoBLE)");
#endif
    Serial.print("Library: DFPongController ");
    Serial.println(DFPONG_CONTROLLER_VERSION);
    if (_settingsLength > SETTINGS_EMPTY_LENGTH) {
        Serial.print("Settings: ");
        Serial.println(_settingsJson);
    }
    Serial.println("Waiting for connection...");
    Serial.println("========================================");
    
    return true;
}

// ============================================
// Main Update Loop
// ============================================

void DFPongController::update() {
    // Nothing to do until begin() has succeeded
    if (!_serviceStarted) return;

#ifndef DFPONG_USE_NIMBLE
    // Process BLE events for ArduinoBLE
    // (NimBLE handles events in its own task via callbacks)
    BLE.poll();
#endif

    unsigned long now = millis();
    
    // A central connected or disconnected since the last update():
    // start this connection with a clean slate
    uint8_t events = _connectionEvents;
    if (events != _seenConnectionEvents) {
        _seenConnectionEvents = events;
        resetState();
    }
    
    // Update status LED
    updateLED();
    
    // Auto-send NEUTRAL if no sendControl() was called in the PREVIOUS loop
    // We check the previous loop because update() runs at the START of each loop,
    // before the user's input code has a chance to call sendControl().
    // This ensures click-based control schemes work correctly:
    // - Loop 1: User clicks → sendControl(UP) → UP is sent
    // - Loop 2: update() sees previous loop had a call, does nothing
    // - Loop 3: update() sees no call in loop 2 → sends NEUTRAL → paddle stops
    // If a click is still waiting for the rate limit (see flushMovement),
    // it is sent first and NEUTRAL follows right after.
    if (!_controlCalledPreviousLoop && isReady()) {
        bool clickWaiting = (_desiredValue != _lastSentValue) && (_desiredValue != NEUTRAL);
        if (!clickWaiting) {
            _desiredValue = NEUTRAL;
        }
    }
    
    // Shift the flags: this loop becomes previous loop for next iteration
    _controlCalledPreviousLoop = _controlCalledThisLoop;
    _controlCalledThisLoop = false;
    
    // Handshake: say hello until the game answers (or give up)
    if (!_handshakeComplete && isConnected()) {
        handleHandshake(now);
    }
    
    // Send a movement value that was held back by the rate limit
    flushMovement(now);
    
    // Send changed settings (at most every 250 ms)
    if (_settingsDirty && (now - _lastSettingsPush >= SETTINGS_UPDATE_INTERVAL)) {
        pushSettings(now);
    }
}

// ============================================
// Handshake
// ============================================

void DFPongController::handleHandshake(unsigned long now) {
    bool subscribed = isSubscribed();
    
    if (subscribed && !_wasSubscribed) {
        _wasSubscribed = true;
        _subscribeTime = now;
        debugPrint("Game subscribed - sending handshake...");
    }
    
    // Send HANDSHAKE right away, then every 500 ms until the game writes it back
    if (subscribed && (_handshakesSent == 0 || now - _lastHandshakeTime >= HANDSHAKE_INTERVAL)) {
        if (notifyMovement(HANDSHAKE)) {
            _lastSentValue = HANDSHAKE;
            _lastNotificationTime = now;
        }
        _lastHandshakeTime = now;
        if (_handshakesSent < 0xFFFF) _handshakesSent++;
    }
    
    // Give up if the game never subscribes or never answers.
    // This frees the controller for the real game if something else connected.
    if (!_disconnectRequested) {
        // _connectionStartTime is set by the connect handler, which may have
        // run after "now" was read, so compare it with a fresh millis()
        unsigned long connectedAt = _connectionStartTime;
        bool timedOut = subscribed
            ? (now - _subscribeTime > HANDSHAKE_TIMEOUT)
            : (millis() - connectedAt > SUBSCRIBE_TIMEOUT);
        if (timedOut) {
            Serial.println("Handshake timeout - disconnecting");
            _disconnectRequested = true;
            disconnectCentral();
        }
    }
}

void DFPongController::disconnectCentral() {
#ifdef DFPONG_USE_NIMBLE
    uint16_t handle = _connHandle;
    if (_pServer != nullptr && handle != NO_CONNECTION) {
        _pServer->disconnect(handle);
    }
#else
    BLE.disconnect();
#endif
}

// ============================================
// LED Control
// ============================================

void DFPongController::updateLED() {
    if (_statusLedPin < 0) return;  // No LED configured
    
    unsigned long currentTime = millis();
    bool connected = isConnected();
    
    if (_handshakeComplete && connected) {
        // Solid ON = ready to play
        digitalWrite(_statusLedPin, HIGH);
    } else if (connected) {
        // Fast blink = connected, handshaking
        if (currentTime - _lastLedToggle >= LED_BLINK_FAST) {
            _ledState = !_ledState;
            digitalWrite(_statusLedPin, _ledState ? HIGH : LOW);
            _lastLedToggle = currentTime;
        }
    } else {
        // Slow blink = disconnected, advertising
        if (currentTime - _lastLedToggle >= LED_BLINK_SLOW) {
            _ledState = !_ledState;
            digitalWrite(_statusLedPin, _ledState ? HIGH : LOW);
            _lastLedToggle = currentTime;
        }
    }
}

// ============================================
// Sending Controls
// ============================================

void DFPongController::sendControl(int direction) {
    // Mark that sendControl was called this loop
    _controlCalledThisLoop = true;
    
    // Validate direction
    if (direction < 0 || direction > 2) {
        direction = NEUTRAL;
    }
    
    // Controls only go out once the game is ready (update() takes care of
    // the handshake until then)
    if (!isReady()) return;
    
    // Remember what the game should have. If it can't go out right now
    // (rate limit), update() sends it as soon as it can.
    _desiredValue = direction;
    flushMovement(millis());
}

void DFPongController::flushMovement(unsigned long now) {
    // Nothing new to send
    if (_desiredValue == _lastSentValue) return;
    
    // Rate limit (update() will try again)
    if (now - _lastNotificationTime < MIN_NOTIFICATION_INTERVAL) return;
    
    // Only send movement once connected, subscribed and handshake complete
    if (!isReady()) return;
    
    if (notifyMovement((uint8_t)_desiredValue)) {
        _lastSentValue = _desiredValue;
        _lastNotificationTime = now;
        
        if (_debug) {
            debugPrint("Sent control", _lastSentValue);
        }
    }
}

bool DFPongController::notifyMovement(uint8_t value) {
    if (_movementCharacteristic == nullptr) return false;

#ifdef DFPONG_USE_NIMBLE
    uint16_t handle = _connHandle;
    if (handle == NO_CONNECTION) return false;
    _movementCharacteristic->setValue(&value, 1);
    return _movementCharacteristic->notify(handle);
#else
    // writeValue() notifies the subscribed game and returns 0 on failure
    return _movementCharacteristic->writeValue((byte)value) > 0;
#endif
}

// ============================================
// Connection Status
// ============================================

bool DFPongController::isConnected() {
#ifdef DFPONG_USE_NIMBLE
    return _serviceStarted && _deviceConnected;
#else
    return _serviceStarted && BLE.connected();
#endif
}

bool DFPongController::isSubscribed() {
#ifdef DFPONG_USE_NIMBLE
    return _subscribed;
#else
    return _movementCharacteristic != nullptr && _movementCharacteristic->subscribed();
#endif
}

bool DFPongController::isReady() {
    return _serviceStarted && _handshakeComplete && isConnected() && isSubscribed();
}

// ============================================
// Signal Strength
// ============================================

int DFPongController::getRSSI() {
    if (!isConnected()) return 0;

#ifdef DFPONG_USE_NIMBLE
    uint16_t handle = _connHandle;
    if (handle == NO_CONNECTION) return 0;
    
    int8_t rssi = 0;
    if (ble_gap_conn_rssi(handle, &rssi) != 0 || rssi == 127) {
        return 0;   // 127 = "not available"
    }
    return rssi;
#else
    BLEDevice central = BLE.central();
    if (central && central.connected()) {
        return central.rssi();
    }
    return 0;
#endif
}

bool DFPongController::hasStrongSignal() {
    int rssi = getRSSI();
    return (rssi != 0) && (rssi > _rssiThreshold);
}

// ============================================
// Information
// ============================================

int DFPongController::getControllerNumber() {
    return _controllerNumber;
}

const char* DFPongController::getServiceUUID() {
    return _serviceUuid;
}

// ============================================
// State Management
// ============================================

// Resets everything about the current connection that update() and
// sendControl() keep track of. Called on every connect / disconnect.
void DFPongController::resetState() {
    _lastSentValue = -1;                // Nothing sent on this connection yet
    _desiredValue = NEUTRAL;
    _controlCalledThisLoop = false;
    _controlCalledPreviousLoop = true;  // Avoid auto-NEUTRAL right after reconnect
    _wasSubscribed = false;
    _disconnectRequested = false;
    _handshakesSent = 0;
    _lastNotificationTime = 0;
    _subscribeTime = 0;
    _lastHandshakeTime = 0;
}

// ============================================
// Player Settings
// ============================================

void DFPongController::setPlayerName(const char* name) {
    if (name == nullptr || name[0] == '\0') {
        removeSetting("name");
        return;
    }
    
    // Count characters (not bytes) so names like "José" are counted correctly,
    // and never cut a character in half.
    const unsigned char* text = (const unsigned char*)name;
    size_t bytes = 0;
    size_t characters = 0;
    while (text[bytes] != '\0' && bytes < MAX_NAME_CHARACTERS * 4) {
        bool startsCharacter = (text[bytes] & 0xC0) != 0x80;
        if (startsCharacter) {
            if (characters == MAX_NAME_CHARACTERS) break;
            characters++;
        }
        bytes++;
    }
    
    if (text[bytes] == '\0') {
        storeSetting("name", name, true);
        return;
    }
    
    // Too long: keep the first 20 characters
    char shortName[MAX_NAME_CHARACTERS * 4 + 1];
    memcpy(shortName, name, bytes);
    shortName[bytes] = '\0';
    
    printWarningStart("setPlayerName(\"");
    Serial.print(name);
    Serial.print("\") is too long (max 20 characters), using \"");
    Serial.print(shortName);
    Serial.println("\"");
    
    storeSetting("name", shortName, true);
}

void DFPongController::setPlayerName(const String& name) {
    setPlayerName(name.c_str());
}

void DFPongController::setPlayerColor(int r, int g, int b) {
    long red = clampInt("setPlayerColor", "red", r, 0, 255);
    long green = clampInt("setPlayerColor", "green", g, 0, 255);
    long blue = clampInt("setPlayerColor", "blue", b, 0, 255);
    
    char color[8];
    color[0] = '#';
    color[1] = HEX_DIGITS[(red >> 4) & 0x0F];
    color[2] = HEX_DIGITS[red & 0x0F];
    color[3] = HEX_DIGITS[(green >> 4) & 0x0F];
    color[4] = HEX_DIGITS[green & 0x0F];
    color[5] = HEX_DIGITS[(blue >> 4) & 0x0F];
    color[6] = HEX_DIGITS[blue & 0x0F];
    color[7] = '\0';
    
    storeSetting("color", color, true);
}

void DFPongController::setPlayerColor(const char* cssColor) {
    if (cssColor == nullptr || cssColor[0] == '\0') {
        removeSetting("color");
        return;
    }
    
    size_t length = strlen(cssColor);
    
    if (cssColor[0] == '#') {
        // "#rgb", "#rgba", "#rrggbb" or "#rrggbbaa"
        bool looksRight = (length == 4 || length == 5 || length == 7 || length == 9) &&
                          isHexText(cssColor + 1);
        if (!looksRight) {
            printWarningStart("setPlayerColor(\"");
            Serial.print(cssColor);
            Serial.println("\") doesn't look like a color code. Use \"#rrggbb\", for example \"#ff8800\".");
        }
        storeSetting("color", cssColor, true);
    } else if (length == 6 && isHexText(cssColor)) {
        // Forgot the "#"? Add it.
        char fixed[8];
        fixed[0] = '#';
        memcpy(fixed + 1, cssColor, 7);   // includes the '\0'
        printWarningStart("setPlayerColor(\"");
        Serial.print(cssColor);
        Serial.print("\"): color codes start with #, using \"");
        Serial.print(fixed);
        Serial.println("\"");
        storeSetting("color", fixed, true);
    } else {
        // Other CSS colors, e.g. "orange" or "rgb(255, 136, 0)"
        storeSetting("color", cssColor, true);
    }
}

void DFPongController::setPlayerColor(const String& cssColor) {
    setPlayerColor(cssColor.c_str());
}

void DFPongController::setMoveSpeed(int speed) {
    storeNumber("moveSpeed", clampInt("setMoveSpeed", nullptr, speed, 1, 100));
}

void DFPongController::setPaddleSize(int percent) {
    storeNumber("paddleSize", clampInt("setPaddleSize", nullptr, percent, 50, 200));
}

void DFPongController::setHitSound(const char* nameOrUrl) {
    setSound("hitSound", "setHitSound", nameOrUrl);
}

void DFPongController::setHitSound(const String& nameOrUrl) {
    setHitSound(nameOrUrl.c_str());
}

void DFPongController::setScoreSound(const char* nameOrUrl) {
    setSound("scoreSound", "setScoreSound", nameOrUrl);
}

void DFPongController::setScoreSound(const String& nameOrUrl) {
    setScoreSound(nameOrUrl.c_str());
}

void DFPongController::setWinSound(const char* nameOrUrl) {
    setSound("winSound", "setWinSound", nameOrUrl);
}

void DFPongController::setWinSound(const String& nameOrUrl) {
    setWinSound(nameOrUrl.c_str());
}

void DFPongController::setSound(const char* key, const char* caller, const char* nameOrUrl) {
    setMedia(key, caller, nameOrUrl, BUILT_IN_SOUNDS, BUILT_IN_SOUND_COUNT, "sound");
}

void DFPongController::setMusic(const char* nameOrUrl) {
    setMedia("music", "setMusic", nameOrUrl, BUILT_IN_MUSIC, BUILT_IN_MUSIC_COUNT, "music track");
}

void DFPongController::setMusic(const String& nameOrUrl) {
    setMusic(nameOrUrl.c_str());
}

// Sounds and music: a name from the game's library, or an https:// link
void DFPongController::setMedia(const char* key, const char* caller, const char* nameOrUrl,
                                const char* const* names, size_t nameCount, const char* kind) {
    if (nameOrUrl == nullptr || nameOrUrl[0] == '\0') {
        removeSetting(key);
        return;
    }
    
    size_t length = strlen(nameOrUrl);
    if (length > MAX_SOUND_LENGTH) {
        printWarningStart(caller);
        Serial.print("(): the text is too long (");
        Serial.print((unsigned long)length);
        Serial.println(" characters, max 200). Not changed.");
        return;
    }
    
    if (strstr(nameOrUrl, "://") != nullptr) {
        // A link to a sound file
        if (startsWithIgnoreCase(nameOrUrl, "http://")) {
            printWarningStart(caller);
            Serial.println("(): the game only loads https:// links. It will try https:// instead - "
                           "use an https:// link to be safe.");
        } else if (!startsWithIgnoreCase(nameOrUrl, "https://")) {
            printWarningStart(caller);
            Serial.println("(): links must start with https://");
        }
    } else {
        // A name from the game's library
        bool builtIn = false;
        for (size_t i = 0; i < nameCount; i++) {
            if (equalsIgnoreCase(nameOrUrl, names[i])) {
                builtIn = true;
                break;
            }
        }
        if (!builtIn) {
            printWarningStart(caller);
            Serial.print("(\"");
            Serial.print(nameOrUrl);
            Serial.print("\"): not a built-in ");
            Serial.print(kind);
            Serial.println(" name. The built-in names are:");
            Serial.print("        ");
            for (size_t i = 0; i < nameCount; i++) {
                if (i > 0) Serial.print(", ");
                Serial.print(names[i]);
            }
            Serial.println();
            Serial.println("        (It still works if a newer version of the game has one with this name.)");
        }
    }
    
    storeSetting(key, nameOrUrl, true);
}

// ============================================
// Game Setting Requests
// ============================================

void DFPongController::requestPointsToWin(int points) {
    storeNumber("pointsToWin", clampInt("requestPointsToWin", nullptr, points, 1, 21));
}

void DFPongController::requestPuckSpeed(float speed) {
    double value = speed;
    if (clampDecimal("requestPuckSpeed", value, 1, 20)) {
        storeDecimal("puckSpeed", value, "requestPuckSpeed");
    }
}

void DFPongController::requestSpeedIncrement(float increment) {
    double value = increment;
    if (clampDecimal("requestSpeedIncrement", value, 0, 1)) {
        storeDecimal("speedIncrement", value, "requestSpeedIncrement");
    }
}

// ============================================
// Any Setting
// ============================================

void DFPongController::setSetting(const char* key, int value) {
    setSetting(key, (long)value);
}

void DFPongController::setSetting(const char* key, unsigned int value) {
    setSetting(key, (unsigned long)value);
}

void DFPongController::setSetting(const char* key, long value) {
    if (!isValidKey(key, "setSetting")) return;
    storeNumber(key, value);
}

void DFPongController::setSetting(const char* key, unsigned long value) {
    if (!isValidKey(key, "setSetting")) return;
    char text[24];
    snprintf(text, sizeof(text), "%lu", value);
    storeSetting(key, text, false);
}

void DFPongController::setSetting(const char* key, double value) {
    if (!isValidKey(key, "setSetting")) return;
    storeDecimal(key, value, "setSetting");
}

void DFPongController::setSetting(const char* key, bool value) {
    if (!isValidKey(key, "setSetting")) return;
    storeSetting(key, value ? "true" : "false", false);
}

void DFPongController::setSetting(const char* key, const char* value) {
    if (!isValidKey(key, "setSetting")) return;
    if (value == nullptr) {
        printWarningStart("setSetting(\"");
        Serial.print(key);
        Serial.println("\"): the value is missing (NULL). Setting not changed.");
        return;
    }
    storeSetting(key, value, true);
}

void DFPongController::setSetting(const char* key, const String& value) {
    setSetting(key, value.c_str());
}

void DFPongController::setSetting(const char* key, const __FlashStringHelper* value) {
    // F("text") strings: on all supported boards these are normal memory
    setSetting(key, reinterpret_cast<const char*>(value));
}

void DFPongController::clearSetting(const char* key) {
    if (!isValidKey(key, "clearSetting")) return;
    removeSetting(key);
}

void DFPongController::clearSettings() {
    if (_settingsLength == SETTINGS_EMPTY_LENGTH) return;   // already empty
    memcpy(_settingsJson, SETTINGS_EMPTY, SETTINGS_EMPTY_LENGTH + 1);
    _settingsLength = SETTINGS_EMPTY_LENGTH;
    _settingsDirty = true;
}

const char* DFPongController::getSettingsJSON() {
    return _settingsJson;
}

// ============================================
// Settings Storage (internal)
// ============================================

bool DFPongController::isValidKey(const char* key, const char* caller) {
    if (key == nullptr || key[0] == '\0') {
        printWarningStart(caller);
        Serial.println("() needs a setting name, for example \"moveSpeed\".");
        return false;
    }
    
    if (strcmp(key, "v") == 0) {
        printWarningStart(caller);
        Serial.println("(\"v\"): \"v\" is used by the library and can't be changed.");
        return false;
    }
    
    size_t length = 0;
    for (const char* k = key; *k; k++) {
        char c = *k;
        bool allowed = isalnum((unsigned char)c) || c == '_' || c == '-';
        length++;
        if (!allowed || length > MAX_KEY_LENGTH) {
            printWarningStart(caller);
            Serial.print("(\"");
            Serial.print(key);
            if (!allowed) {
                Serial.println("\"): setting names can only use letters, numbers, _ and -");
            } else {
                Serial.println("\"): setting names can be at most 32 characters");
            }
            return false;
        }
    }
    return true;
}

// Finds ,"key":value in the JSON. start/end mark the whole entry
// (from the comma up to, not including, the next comma or closing brace).
bool DFPongController::findSetting(const char* key, uint16_t& start, uint16_t& end) {
    size_t keyLength = strlen(key);
    uint16_t pos = SETTINGS_HEADER_LENGTH;
    
    while (pos < _settingsLength && _settingsJson[pos] == ',') {
        uint16_t entryStart = pos;
        pos += 2;                                   // skip ,"
        uint16_t keyStart = pos;
        while (pos < _settingsLength && _settingsJson[pos] != '"') pos++;
        uint16_t keyEnd = pos;
        pos += 2;                                   // skip ":
        
        if (pos < _settingsLength && _settingsJson[pos] == '"') {
            // Text value: skip to the closing quote (escaped quotes don't count)
            pos++;
            while (pos < _settingsLength && _settingsJson[pos] != '"') {
                if (_settingsJson[pos] == '\\') pos++;
                pos++;
            }
            pos++;                                  // closing quote
        } else {
            // Number / true / false
            while (pos < _settingsLength && _settingsJson[pos] != ',' && _settingsJson[pos] != '}') pos++;
        }
        
        if ((size_t)(keyEnd - keyStart) == keyLength &&
            memcmp(&_settingsJson[keyStart], key, keyLength) == 0) {
            start = entryStart;
            end = pos;
            return true;
        }
    }
    return false;
}

// Adds or replaces one setting. "value" is either text (quoted and escaped)
// or a ready-made JSON number / true / false.
bool DFPongController::storeSetting(const char* key, const char* value, bool isText) {
    size_t newLength = emitEntry(nullptr, EMIT_COUNT, nullptr, key, value, isText);
    
    uint16_t start;
    uint16_t end;
    bool found = findSetting(key, start, end);
    if (!found) {
        // New setting: insert before the closing brace
        start = (uint16_t)(_settingsLength - 1);
        end = start;
    }
    size_t oldLength = (size_t)(end - start);
    
    // Same value as before? Nothing to do (avoids needless updates to the game)
    if (found && oldLength == newLength) {
        bool same = true;
        emitEntry(&_settingsJson[start], EMIT_COMPARE, &same, key, value, isText);
        if (same) return true;
    }
    
    size_t total = _settingsLength - oldLength + newLength;
    if (total > SETTINGS_MAX_LENGTH) {
        Serial.print("DFPong: can't save setting \"");
        Serial.print(key);
        Serial.print("\" - all settings together must fit in 512 bytes (this would make ");
        Serial.print((unsigned long)total);
        Serial.println("). Use shorter text or clear a setting you don't need.");
        return false;
    }
    
    // Make room (or close the gap), then write the new entry in place.
    // The tail includes the closing brace and the '\0'.
    memmove(&_settingsJson[start + newLength], &_settingsJson[end], _settingsLength - end + 1);
    emitEntry(&_settingsJson[start], EMIT_WRITE, nullptr, key, value, isText);
    _settingsLength = (uint16_t)total;
    _settingsDirty = true;
    return true;
}

bool DFPongController::storeNumber(const char* key, long value) {
    char text[24];
    snprintf(text, sizeof(text), "%ld", value);
    return storeSetting(key, text, false);
}

bool DFPongController::storeDecimal(const char* key, double value, const char* caller) {
    char text[24];
    if (!formatDecimal(value, text, sizeof(text))) {
        printWarningStart(caller);
        Serial.print("(\"");
        Serial.print(key);
        Serial.println("\"): the value must be a normal number (not NaN/infinity, smaller than 100 trillion).");
        return false;
    }
    return storeSetting(key, text, false);
}

bool DFPongController::removeSetting(const char* key) {
    uint16_t start;
    uint16_t end;
    if (!findSetting(key, start, end)) return false;
    
    memmove(&_settingsJson[start], &_settingsJson[end], _settingsLength - end + 1);
    _settingsLength = (uint16_t)(_settingsLength - (end - start));
    _settingsDirty = true;
    return true;
}

long DFPongController::clampInt(const char* caller, const char* part, long value,
                                long minValue, long maxValue) {
    long used = value;
    if (used < minValue) used = minValue;
    if (used > maxValue) used = maxValue;
    
    if (used != value) {
        // e.g. "DFPong: setMoveSpeed(500) is out of range (1-100), using 100"
        printWarningStart(caller);
        Serial.print("(");
        if (part != nullptr) {
            Serial.print(part);
            Serial.print(" = ");
        }
        Serial.print(value);
        Serial.print(") is out of range (");
        Serial.print(minValue);
        Serial.print("-");
        Serial.print(maxValue);
        Serial.print("), using ");
        Serial.println(used);
    }
    return used;
}

bool DFPongController::clampDecimal(const char* caller, double& value,
                                    double minValue, double maxValue) {
    if (value != value) {   // NaN
        printWarningStart(caller);
        Serial.println("(): the value is not a number. Setting not changed.");
        return false;
    }
    
    double used = value;
    if (used < minValue) used = minValue;
    if (used > maxValue) used = maxValue;
    
    if (used != value) {
        printWarningStart(caller);
        Serial.print("(");
        printDecimal(value);
        Serial.print(") is out of range (");
        printDecimal(minValue);
        Serial.print("-");
        printDecimal(maxValue);
        Serial.print("), using ");
        printDecimal(used);
        Serial.println();
        value = used;
    }
    return true;
}

void DFPongController::pushSettings(unsigned long now) {
    if (_configCharacteristic == nullptr) return;
    
    // The game re-reads the whole value when it gets the notification
    writeConfigValue();
#ifdef DFPONG_USE_NIMBLE
    _configCharacteristic->notify();
#endif

    _settingsDirty = false;
    _lastSettingsPush = now;
    
    if (_debug) {
        Serial.print("Settings sent: ");
        Serial.println(_settingsJson);
    }
}

// Puts the settings JSON into the config characteristic. Trailing spaces
// (still valid JSON) keep the length off packet boundaries; see isPacketBoundary().
// On ArduinoBLE this also notifies a subscribed game.
void DFPongController::writeConfigValue() {
    if (_configCharacteristic == nullptr) return;
    
    uint16_t length = _settingsLength;
    while (isPacketBoundary(length) && length < SETTINGS_MAX_LENGTH) {
        _settingsJson[length++] = ' ';
    }
    
#ifdef DFPONG_USE_NIMBLE
    _configCharacteristic->setValue((const uint8_t*)_settingsJson, length);
#else
    _configCharacteristic->writeValue((const uint8_t*)_settingsJson, (int)length);
#endif
    
    // Both stacks copied the value, so restore the stored JSON
    _settingsJson[_settingsLength] = '\0';
}

// ============================================
// Debug Helpers
// ============================================

void DFPongController::debugPrint(const char* message) {
    if (_debug) {
        Serial.println(message);
    }
}

void DFPongController::debugPrint(const char* message, int value) {
    if (_debug) {
        Serial.print(message);
        Serial.print(": ");
        Serial.println(value);
    }
}

// ============================================
// ArduinoBLE Event Handlers (Arduino only)
// ============================================
// These run inside BLE.poll(), i.e. inside update().

#ifndef DFPONG_USE_NIMBLE

void DFPongController::onBLEConnected(BLEDevice central) {
    if (_instance == nullptr) return;
    
    Serial.print("Connected to: ");
    Serial.println(central.address());
    
    // update() resets the rest of the state for the new connection
    _instance->_connectionStartTime = millis();
    _instance->_handshakeComplete = false;
    _instance->_deviceConnected = true;
    _instance->_connectionEvents = (uint8_t)(_instance->_connectionEvents + 1);
}

void DFPongController::onBLEDisconnected(BLEDevice central) {
    if (_instance == nullptr) return;
    
    Serial.print("Disconnected from: ");
    Serial.println(central.address());
    Serial.println("Waiting for connection...");
    
    // update() resets the rest of the state
    _instance->_deviceConnected = false;
    _instance->_handshakeComplete = false;
    _instance->_connectionEvents = (uint8_t)(_instance->_connectionEvents + 1);
    
    // Ensure clean advertising restart
    BLE.stopAdvertise();
    delay(50);
    BLE.advertise();
}

void DFPongController::onCharacteristicWritten(BLEDevice, BLECharacteristic characteristic) {
    if (_instance == nullptr) return;
    
    // Ignore empty writes
    if (characteristic.valueLength() < 1) return;
    
    uint8_t value = characteristic.value()[0];
    
    if (value == HANDSHAKE && !_instance->_handshakeComplete) {
        _instance->_handshakeComplete = true;
        _instance->debugPrint("Handshake complete!");
        Serial.println("Controller ready to play!");
    }
}

#endif // !DFPONG_USE_NIMBLE
