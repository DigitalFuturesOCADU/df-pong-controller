// Minimal ArduinoBLE mock: characteristics keep shared state, writes can notify
#pragma once
#include "Arduino.h"
#include <memory>
enum { BLERead = 0x02, BLEWriteWithoutResponse = 0x04, BLEWrite = 0x08, BLENotify = 0x10 };
enum { BLEConnected = 0, BLEDisconnected = 1, BLEWritten = 2 };
class BLEDevice {
public:
    explicit BLEDevice(bool c = false) : c_(c) {}
    explicit operator bool() const { return c_; }
    bool connected() const { return c_; }
    int rssi() const { return -50; }
    const char* address() const { return "aa:bb:cc:dd:ee:ff"; }
private:
    bool c_;
};
class BLECharacteristic;
typedef void (*BLEDeviceEventHandler)(BLEDevice);
typedef void (*BLECharacteristicEventHandler)(BLEDevice, BLECharacteristic);
struct Notification { unsigned long time; std::string uuid; std::vector<uint8_t> data; };
extern std::vector<Notification> g_notifications;
extern bool g_centralConnected;
struct CharState { std::string uuid; uint16_t props; int size; std::vector<uint8_t> value; bool subscribed = false; BLECharacteristicEventHandler written = nullptr; };
class BLECharacteristic {
public:
    BLECharacteristic(const char* uuid, uint16_t props, int size) : st_(new CharState()) { st_->uuid = uuid; st_->props = props; st_->size = size > 512 ? 512 : size; }
    int writeValue(const uint8_t* v, int len, bool = true) {
        if (len > st_->size) len = st_->size;
        st_->value.assign(v, v + len);
        if ((st_->props & BLENotify) && st_->subscribed) { if (!g_centralConnected) return 0; g_notifications.push_back({g_now, st_->uuid, st_->value}); }
        return len;
    }
    int valueLength() const { return (int)st_->value.size(); }
    const uint8_t* value() const { return st_->value.data(); }
    bool subscribed() { return st_->subscribed; }
    void setEventHandler(int ev, BLECharacteristicEventHandler h) { if (ev == BLEWritten) st_->written = h; }
    const char* uuid() const { return st_->uuid.c_str(); }
    std::shared_ptr<CharState> st_;
};
class BLEByteCharacteristic : public BLECharacteristic {
public:
    BLEByteCharacteristic(const char* uuid, uint16_t props) : BLECharacteristic(uuid, props, 1) {}
    int writeValue(byte v) { return BLECharacteristic::writeValue(&v, 1); }
};
class BLEService {
public:
    explicit BLEService(const char* u) : uuid_(u) {}
    const char* uuid() const { return uuid_.c_str(); }
    void addCharacteristic(BLECharacteristic& c) { chars.push_back(c); }
    std::vector<BLECharacteristic> chars;
private:
    std::string uuid_;
};
class MockBLE {
public:
    std::vector<BLEService*> services;
    BLEDeviceEventHandler onConnectH = nullptr, onDisconnectH = nullptr;
    int begin() { return 1; }
    void poll() {}
    bool connected() { return g_centralConnected; }
    BLEDevice central() { return BLEDevice(g_centralConnected); }
    void disconnect() {}
    void stopAdvertise() {}
    int advertise() { return 1; }
    void setEventHandler(int ev, BLEDeviceEventHandler h) { if (ev == BLEConnected) onConnectH = h; else if (ev == BLEDisconnected) onDisconnectH = h; }
    void setLocalName(const char*) {}
    void setAdvertisedServiceUuid(const char*) {}
    void setConnectionInterval(int, int) {}
    void setPairable(bool) {}
    void setAdvertisingInterval(int) {}
    void setManufacturerData(const uint8_t*, int) {}
    void addService(BLEService& s) { services.push_back(&s); }
};
extern MockBLE BLE;
