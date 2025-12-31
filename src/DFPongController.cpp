/*
 * DFPongController.cpp
 * 
 * Implementation of the DFPongController library.
 * Supports both ArduinoBLE (Arduino boards) and NimBLE (ESP32).
 * 
 * Created by Digital Futures OCAD U
 * MIT License
 */

#include "DFPongController.h"

// ============================================
// Static Members
// ============================================

// Singleton instance pointer for callbacks
DFPongController* DFPongController::_instance = nullptr;

// Manufacturer data: 0xDF = DFPong, 0x01 = version 1
const uint8_t DFPongController::MANUFACTURER_DATA[2] = {0xDF, 0x01};

// ============================================
// NimBLE Callback Classes (ESP32 only)
// ============================================

#ifdef DFPONG_USE_NIMBLE

class DFPongController::ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        if (DFPongController::_instance == nullptr) return;
        
        Serial.print("Connected to: ");
        Serial.println(connInfo.getAddress().toString().c_str());
        
        DFPongController::_instance->_deviceConnected = true;
        DFPongController::_instance->_handshakeComplete = false;
        DFPongController::_instance->_lastSentValue = HANDSHAKE;
        DFPongController::_instance->_valueChanged = true;
        DFPongController::_instance->_connectionStartTime = millis();
        
        if (DFPongController::_instance->_statusLedPin >= 0) {
            digitalWrite(DFPongController::_instance->_statusLedPin, HIGH);
        }
    }
    
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        if (DFPongController::_instance == nullptr) return;
        
        Serial.print("Disconnected from: ");
        Serial.println(connInfo.getAddress().toString().c_str());
        Serial.println("Waiting for connection...");
        
        DFPongController::_instance->_deviceConnected = false;
        DFPongController::_instance->resetState();
        
        // Restart advertising
        NimBLEDevice::startAdvertising();
    }
};

class DFPongController::CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        if (DFPongController::_instance == nullptr) return;
        
        uint8_t value = pCharacteristic->getValue()[0];
        
        if (value == HANDSHAKE) {
            DFPongController::_instance->_handshakeComplete = true;
            DFPongController::_instance->debugPrint("Handshake complete!");
            Serial.println("Controller ready to play!");
        }
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
    
    _serviceStarted = false;
    _handshakeComplete = false;
    _deviceConnected = false;
    _ledState = false;
    _lastSentValue = 0;
    _valueChanged = false;
    
    _lastLedToggle = 0;
    _lastNotificationTime = 0;
    _connectionStartTime = 0;
    
    // Set singleton instance
    _instance = this;
}

// ============================================
// Configuration Methods
// ============================================

void DFPongController::setControllerNumber(int number) {
    _controllerNumber = number;
    
    if (_debug) {
        Serial.print("Controller number set to: ");
        Serial.println(number);
    }
}

void DFPongController::setStatusLED(int pin) {
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
    
    // Calculate unique suffix: device 1 -> 14 (0x0e), device 2 -> 15 (0x0f), etc.
    int suffix = 13 + _controllerNumber;
    
    // Generate full UUIDs with 2-digit hex suffix
    snprintf(_serviceUuid, sizeof(_serviceUuid), "%s%02x", serviceBase, suffix);
    snprintf(_characteristicUuid, sizeof(_characteristicUuid), "%s%02x", characteristicBase, suffix);
    
    if (_debug) {
        Serial.print("Service UUID: ");
        Serial.println(_serviceUuid);
        Serial.print("Characteristic UUID: ");
        Serial.println(_characteristicUuid);
    }
}

// ============================================
// Initialization
// ============================================

bool DFPongController::begin() {
    // Generate default device name
    char defaultName[16];
    snprintf(defaultName, sizeof(defaultName), "DFPONG-%d", _controllerNumber);
    return begin(defaultName);
}

bool DFPongController::begin(const char* deviceName) {
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

#ifdef DFPONG_USE_NIMBLE
    // ========== NimBLE (ESP32) Implementation ==========
    
    debugPrint("Starting NimBLE...");
    
    // Initialize NimBLE
    NimBLEDevice::init(deviceName);
    
    // Set power level for better range
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    
    // Create server
    _pServer = NimBLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks());
    
    // Create service
    _pongService = _pServer->createService(_serviceUuid);
    
    // Create characteristic with read, write, notify
    _movementCharacteristic = _pongService->createCharacteristic(
        _characteristicUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    _movementCharacteristic->setCallbacks(new CharacteristicCallbacks());
    _movementCharacteristic->setValue((uint8_t*)"\0", 1);
    
    // Start the service
    _pongService->start();
    
    // Configure advertising
    _pAdvertising = NimBLEDevice::getAdvertising();
    _pAdvertising->addServiceUUID(_serviceUuid);
    _pAdvertising->setScanResponse(true);
    _pAdvertising->setMinPreferred(0x06);  // For iPhone compatibility
    _pAdvertising->setMaxPreferred(0x12);
    
    // Start advertising
    NimBLEDevice::startAdvertising();
    
#else
    // ========== ArduinoBLE Implementation ==========
    
    // Create BLE service and characteristic
    _pongService = new BLEService(_serviceUuid);
    _movementCharacteristic = new BLEByteCharacteristic(
        _characteristicUuid, 
        BLERead | BLENotify | BLEWrite
    );
    
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
    
    // Add characteristic to service and service to BLE
    _pongService->addCharacteristic(*_movementCharacteristic);
    BLE.addService(*_pongService);
    
    // Set initial value
    _movementCharacteristic->writeValue(0);
    delay(100);
    
    // Start advertising
    BLE.advertise();
    
#endif // DFPONG_USE_NIMBLE

    _serviceStarted = true;
    
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
    Serial.println("Waiting for connection...");
    Serial.println("========================================");
    
    return true;
}

// ============================================
// Main Update Loop
// ============================================

void DFPongController::update() {
#ifdef DFPONG_USE_NIMBLE
    // NimBLE handles events automatically via callbacks
    // Just update LED and check handshake timeout
#else
    // Process BLE events for ArduinoBLE
    BLE.poll();
#endif
    
    // Update status LED
    updateLED();
    
    // Check for handshake timeout
    if (isConnected() && !_handshakeComplete) {
        if (millis() - _connectionStartTime > HANDSHAKE_TIMEOUT) {
            debugPrint("Handshake timeout - disconnecting");
#ifdef DFPONG_USE_NIMBLE
            _pServer->disconnect(0);
#else
            BLE.disconnect();
#endif
        }
    }
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
            digitalWrite(_statusLedPin, _ledState);
            _lastLedToggle = currentTime;
        }
    } else {
        // Slow blink = disconnected, advertising
        if (currentTime - _lastLedToggle >= LED_BLINK_SLOW) {
            _ledState = !_ledState;
            digitalWrite(_statusLedPin, _ledState);
            _lastLedToggle = currentTime;
        }
    }
}

// ============================================
// Sending Controls
// ============================================

void DFPongController::sendControl(int direction) {
    // Validate direction
    if (direction < 0 || direction > 2) {
        direction = NEUTRAL;
    }
    
#ifdef DFPONG_USE_NIMBLE
    // Can't send if not connected
    if (!_deviceConnected) {
        return;
    }
#else
    // Can't send if not connected or subscribed
    if (!BLE.connected() || !_movementCharacteristic->subscribed()) {
        return;
    }
#endif
    
    // If handshake not complete, keep sending handshake signal
    int valueToSend = _handshakeComplete ? direction : HANDSHAKE;
    
    // Check if value changed
    if (valueToSend != _lastSentValue) {
        _valueChanged = true;
    }
    
    // Only send if value changed and enough time has passed
    unsigned long currentTime = millis();
    if (_valueChanged && (currentTime - _lastNotificationTime >= MIN_NOTIFICATION_INTERVAL)) {
#ifdef DFPONG_USE_NIMBLE
        uint8_t val = (uint8_t)valueToSend;
        _movementCharacteristic->setValue(&val, 1);
        _movementCharacteristic->notify();
        _lastSentValue = valueToSend;
        _lastNotificationTime = currentTime;
        _valueChanged = false;
        
        if (_debug && valueToSend != HANDSHAKE) {
            debugPrint("Sent control", valueToSend);
        }
#else
        if (_movementCharacteristic->writeValue(valueToSend)) {
            _lastSentValue = valueToSend;
            _lastNotificationTime = currentTime;
            _valueChanged = false;
            
            if (_debug && valueToSend != HANDSHAKE) {
                debugPrint("Sent control", valueToSend);
            }
        }
#endif
    }
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

bool DFPongController::isReady() {
#ifdef DFPONG_USE_NIMBLE
    return _serviceStarted && _deviceConnected && _handshakeComplete;
#else
    return _serviceStarted && BLE.connected() && 
           _movementCharacteristic->subscribed() && _handshakeComplete;
#endif
}

// ============================================
// Signal Strength
// ============================================

int DFPongController::getRSSI() {
#ifdef DFPONG_USE_NIMBLE
    if (_pServer && _pServer->getConnectedCount() > 0) {
        // NimBLE doesn't easily expose per-connection RSSI
        // Return a placeholder - would need more complex implementation
        return -50;  // Approximate value
    }
    return 0;
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

void DFPongController::resetState() {
    _handshakeComplete = false;
    _lastSentValue = 0;
    _valueChanged = false;
    _lastNotificationTime = 0;
    _connectionStartTime = 0;
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

#ifndef DFPONG_USE_NIMBLE

void DFPongController::onBLEConnected(BLEDevice central) {
    if (_instance == nullptr) return;
    
    Serial.print("Connected to: ");
    Serial.println(central.address());
    
    // Reset state for new connection
    _instance->_deviceConnected = true;
    _instance->_handshakeComplete = false;
    _instance->_lastSentValue = HANDSHAKE;
    _instance->_valueChanged = true;
    _instance->_connectionStartTime = millis();
    
    // LED solid during handshake (updateLED will handle blinking)
    if (_instance->_statusLedPin >= 0) {
        digitalWrite(_instance->_statusLedPin, HIGH);
    }
}

void DFPongController::onBLEDisconnected(BLEDevice central) {
    if (_instance == nullptr) return;
    
    Serial.print("Disconnected from: ");
    Serial.println(central.address());
    Serial.println("Waiting for connection...");
    
    // Reset all state
    _instance->_deviceConnected = false;
    _instance->resetState();
    
    // Ensure clean advertising restart
    BLE.stopAdvertise();
    delay(50);
    BLE.advertise();
}

void DFPongController::onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance == nullptr) return;
    
    byte value = _instance->_movementCharacteristic->value();
    
    if (value == HANDSHAKE) {
        _instance->_handshakeComplete = true;
        _instance->debugPrint("Handshake complete!");
        Serial.println("Controller ready to play!");
    }
}

#endif // !DFPONG_USE_NIMBLE
