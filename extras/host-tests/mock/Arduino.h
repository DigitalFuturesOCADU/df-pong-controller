// Minimal Arduino mock for host tests of DFPongController
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <vector>
typedef uint8_t byte;
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
#define LED_BUILTIN 13
extern unsigned long g_now;
inline unsigned long millis() { return g_now; }
inline void delay(unsigned long ms) { g_now += ms; }
struct ToneCall { unsigned long time; int pin; unsigned int freq; unsigned long ms; };
extern std::vector<ToneCall> g_tones;
extern int g_ledLevel;
inline void pinMode(int, int) {}
inline void digitalWrite(int pin, int v) { if (pin == LED_BUILTIN) g_ledLevel = v; }
inline void tone(int pin, unsigned int f, unsigned long ms = 0) { g_tones.push_back({g_now, pin, f, ms}); }
inline void noTone(int pin) { g_tones.push_back({g_now, pin, 0, 0}); }
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper*>(x))
class String {
public:
    String(const char* s = "") : s_(s ? s : "") {}
    const char* c_str() const { return s_.c_str(); }
    unsigned int length() const { return (unsigned int)s_.size(); }
private:
    std::string s_;
};
struct MockSerial {
    std::string out;
    void begin(long) {}
    explicit operator bool() const { return true; }
    void print(const char* s) { out += s; }
    void print(const String& s) { out += s.c_str(); }
    void print(char c) { out += c; }
    void print(const __FlashStringHelper* s) { out += reinterpret_cast<const char*>(s); }
    void print(int v) { out += std::to_string(v); }
    void print(unsigned int v) { out += std::to_string(v); }
    void print(long v) { out += std::to_string(v); }
    void print(unsigned long v) { out += std::to_string(v); }
    void print(double v) { char b[32]; snprintf(b, sizeof b, "%.2f", v); out += b; }
    void println() { out += "\n"; }
    template <typename T> void println(T v) { print(v); out += "\n"; }
};
extern MockSerial Serial;
