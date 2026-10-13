// Generated from api/dfpong-api.json by api/build.mjs. Edit the spec, not this file.
//
// The DFPONG API 1.0: the limits and names the game and this library agree on.
// The game also accepts names this library doesn't know yet, so a newer game can add sounds
// and music without a new library release.

#ifndef DFPONG_SPEC_H
#define DFPONG_SPEC_H

#include <stddef.h>

#define DFPONG_API_VERSION "1.0"
#define DFPONG_CONFIG_VERSION 1

namespace DFPongSpec {

    // Bluetooth
    const char* const SERVICE_UUID_BASE = "19b10010-e8f2-537e-4f6c-d104768a12";
    const char* const MOVEMENT_UUID_BASE = "19b10011-e8f2-537e-4f6c-d104768a12";
    const char* const CONFIG_UUID_BASE = "19b10012-e8f2-537e-4f6c-d104768a12";
    const int CONTROLLER_NUMBER_MIN = 1;
    const int CONTROLLER_NUMBER_MAX = 242;
    const size_t CONFIG_MAX_BYTES = 512;
    const unsigned long CONFIG_MIN_UPDATE_MS = 250;
    const unsigned long HANDSHAKE_REPEAT_MS = 500; // how often a controller repeats its hello until the game answers
    const unsigned long SUBSCRIBE_TIMEOUT_MS = 10000; // how long a controller waits for the game to subscribe after connecting
    const unsigned long HANDSHAKE_TIMEOUT_MS = 5000; // how long a controller waits for the handshake after the game subscribes
    const unsigned long ALIVE_CHECK_MS = 1500; // how often the game checks a connected controller still answers
    const unsigned long ALIVE_TIMEOUT_MS = 2000; // how long the controller has to answer a check
    const unsigned long ALIVE_STRIKES = 2; // missed checks in a row before the game reconnects (and pauses a match)

    // Settings
    const size_t NAME_MAX_CHARACTERS = 20;
    const size_t MEDIA_MAX_LENGTH = 200; // sound and music names or links
    // Move speed
    const long MOVE_SPEED_MIN = 1;
    const long MOVE_SPEED_MAX = 100;
    const long MOVE_SPEED_DEFAULT = 10;
    // Paddle size
    const long PADDLE_SIZE_MIN = 50;
    const long PADDLE_SIZE_MAX = 200;
    const long PADDLE_SIZE_DEFAULT = 100;
    // Points to win
    const long POINTS_TO_WIN_MIN = 1;
    const long POINTS_TO_WIN_MAX = 21;
    const long POINTS_TO_WIN_DEFAULT = 10;
    // Puck speed
    const double PUCK_SPEED_MIN = 1.0;
    const double PUCK_SPEED_MAX = 20.0;
    const double PUCK_SPEED_DEFAULT = 5.0;
    // Speed-up per hit
    const double SPEED_INCREMENT_MIN = 0.0;
    const double SPEED_INCREMENT_MAX = 1.0;
    const double SPEED_INCREMENT_DEFAULT = 0.15;

    // Built-in sound names (hitSound, scoreSound, winSound)
    const char* const SOUNDS[] = {
        "pingpong",
        "claves",
        "tick",
        "bell",
        "gong",
        "bikebell",
        "flourish",
        "applause",
        "pip",
        "pipgo",
        "quindar",
        "clock",
        "blip",
        "boop",
        "pluck",
        "click",
        "laser",
        "drum",
        "chirp",
        "zap",
        "thud",
        "wood",
        "glass",
        "coin",
        "classic",
        "wall",
        "ding",
    };
    const size_t SOUND_COUNT = sizeof(SOUNDS) / sizeof(SOUNDS[0]);

    // Built-in music names
    const char* const MUSIC[] = {
        "drift",
        "tabletop",
        "arcade",
        "nightbus",
        "gymnopedie",
        "smallsteps",
        "bounce",
        "currents",
        "palmleaf",
        "spacemtv",
    };
    const size_t MUSIC_COUNT = sizeof(MUSIC) / sizeof(MUSIC[0]);

}  // namespace DFPongSpec

#endif
