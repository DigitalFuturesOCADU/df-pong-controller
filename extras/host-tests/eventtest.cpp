// Host tests: game events and the buzzer (ArduinoBLE path, mocked)
#include "DFPongController.h"
#include <functional>

unsigned long g_now = 0;
int g_ledLevel = 0;
MockSerial Serial;
MockBLE BLE;
std::vector<Notification> g_notifications;
std::vector<ToneCall> g_tones;
bool g_centralConnected = false;

static int g_failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s  (line %d)\n", msg, __LINE__); g_failures++; } else { printf("ok:   %s\n", msg); } } while (0)

DFPongController controller;

static BLECharacteristic* findChar(const std::string& uuid) {
    for (BLEService* s : BLE.services) for (BLECharacteristic& c : s->chars) if (c.st_->uuid == uuid) return &c;
    return nullptr;
}
static std::string uuidFor(char kind) { std::string u = controller.getServiceUUID(); u[7] = kind; return u; }
static void connectGame() {
    g_centralConnected = true; BLE.onConnectH(BLEDevice(true));
    for (char k : {'1', '2', '3'}) findChar(uuidFor(k))->st_->subscribed = true;
    for (int i = 0; i < 20; i++) { controller.update(); g_now++; }
    BLECharacteristic* m = findChar(uuidFor('1')); uint8_t v = 3; m->writeValue(&v, 1); m->st_->written(BLEDevice(true), *m);
}
static void disconnectGame() {
    g_centralConnected = false;
    for (BLEService* s : BLE.services) for (BLECharacteristic& c : s->chars) c.st_->subscribed = false;
    BLE.onDisconnectH(BLEDevice(false));
}
static void gameSends(std::vector<uint8_t> bytes) {      // the game writes to the events characteristic
    BLECharacteristic* e = findChar(uuidFor('4'));
    e->writeValue(bytes.data(), (int)bytes.size());
    if (e->st_->written) e->st_->written(BLEDevice(true), *e);
}
static void run(unsigned long ms) { unsigned long end = g_now + ms; while (g_now < end) { controller.update(); g_now++; } }
static std::vector<unsigned> tonesSince(size_t from) { std::vector<unsigned> f; for (size_t i = from; i < g_tones.size(); i++) if (g_tones[i].freq) f.push_back(g_tones[i].freq); return f; }

static std::vector<std::pair<int, int>> handled;
static void handler(int event, int value) { handled.push_back({event, value}); }

int main() {
    controller.setControllerNumber(9);
    controller.setBuzzer(11);
    size_t t0 = 0;
    run(400);
    auto hello = tonesSince(t0);
    CHECK(hello.size() == 3 && hello[0] == 523 && hello[1] == 659 && hello[2] == 784, "setBuzzer plays C E G, one note after another, before begin()");
    CHECK(g_tones.back().freq == 0, "the buzzer goes quiet after the jingle");

    controller.begin();
    BLECharacteristic* ev = findChar(uuidFor('4'));
    CHECK(ev != nullptr && (ev->st_->props & BLEWrite) && (ev->st_->props & BLEWriteWithoutResponse) && !(ev->st_->props & BLENotify), "events characteristic (19b10014-...): write and write-without-response, no notify");

    size_t t1 = g_tones.size();
    connectGame();
    run(300);
    auto ready = tonesSince(t1);
    CHECK(ready.size() == 2 && ready[0] == 784 && ready[1] == 1047, "two rising notes when the game is ready");

    // No handler: events wait for readGameEvent()
    gameSends({4, 3, 1});           // scored 3-1
    gameSends({3, 200});            // hit
    run(5);
    int e1 = controller.readGameEvent(); int v1 = controller.gameEventValue();
    int e2 = controller.readGameEvent(); int v2 = controller.gameEventValue();
    CHECK(e1 == EVENT_SCORED && v1 == 3 && e2 == EVENT_HIT && v2 == 200, "readGameEvent returns events in order, with their values");
    CHECK(controller.readGameEvent() == NO_EVENT, "then NO_EVENT");
    CHECK(controller.myScore() == 3 && controller.theirScore() == 1, "score kept from EVENT_SCORED");
    gameSends({5, 2, 3});           // conceded: they have 2, you have 3
    run(5);
    CHECK(controller.myScore() == 3 && controller.theirScore() == 2, "score kept from EVENT_CONCEDED");
    controller.readGameEvent();

    // Handler
    controller.onGameEvent(handler);
    gameSends({2, 3}); gameSends({2, 0}); gameSends({1});
    run(5);
    CHECK(handled.size() == 3 && handled[0] == std::make_pair(2, 3) && handled[1] == std::make_pair(2, 0) && handled[2].first == EVENT_MATCH_START, "onGameEvent gets every event with its value");
    CHECK(controller.readGameEvent() == NO_EVENT, "with a handler, nothing waits for readGameEvent");
    CHECK(controller.myScore() == 0 && controller.theirScore() == 0, "EVENT_MATCH_START resets the score");

    // Sounds and priorities
    run(500);
    size_t t2 = g_tones.size();
    gameSends({2, 0});
    run(5);
    CHECK(tonesSince(t2).size() == 1 && tonesSince(t2)[0] == 1760, "GO is a high note");
    run(300);
    size_t t3 = g_tones.size();
    gameSends({3, 255});
    run(5);
    CHECK(tonesSince(t3).size() == 1 && tonesSince(t3)[0] == 700 + 255 * 3, "a hit's pitch rises with puck speed");
    run(100);
    size_t t4 = g_tones.size();
    gameSends({7});                 // won
    run(100);
    gameSends({3, 0});              // a hit during the fanfare
    run(1200);
    auto won = tonesSince(t4);
    bool hitSlipped = false; for (unsigned f : won) if (f == 700) hitSlipped = true;
    CHECK(won.size() == 6 && won[0] == 523 && won.back() == 1047 && !hitSlipped, "the winning fanfare plays in full; a hit doesn't cut it off");
    controller.playTone(440, 50);
    run(5);
    CHECK(g_tones.back().freq == 440 && g_tones.back().ms == 50, "playTone plays your own note");

    // Robustness
    gameSends({});                  // empty write
    gameSends({99, 1, 2, 3, 4});    // an event from a newer game
    run(5);
    CHECK(handled.back().first == 99, "unknown events still reach the sketch (no sound, no crash)");
    for (int i = 0; i < 30; i++) gameSends({3, 10});
    run(5);
    CHECK(true, "30 events at once (more than the queue holds) are handled without a crash");

    // Dropped
    size_t t5 = g_tones.size();
    disconnectGame();
    run(400);
    auto dropped = tonesSince(t5);
    CHECK(dropped.size() == 2 && dropped[0] == 523 && dropped[1] == 392, "two falling notes when the connection drops");

    // No buzzer
    controller.setBuzzer(-1);
    size_t t6 = g_tones.size();
    connectGame();
    gameSends({7});
    run(1500);
    CHECK(tonesSince(t6).empty(), "setBuzzer(-1): no sounds at all");

    // Game sounds off, own sounds on
    controller.setBuzzer(11, false);
    run(400);
    size_t t7 = g_tones.size();
    gameSends({4, 1, 0});
    run(400);
    CHECK(tonesSince(t7).empty(), "setBuzzer(pin, false): no game-event sounds");
    controller.playTone(660, 40);
    CHECK(g_tones.back().freq == 660, "... but playTone still works");

    printf("\n%d failure(s)\n", g_failures);
    return g_failures != 0;
}
