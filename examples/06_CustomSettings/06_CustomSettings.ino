/*
 * =============================================================================
 * DFPongController - Custom Settings Example
 * =============================================================================
 *
 * This example is a normal two-button controller (like 01_SimpleDigital)
 * that ALSO personalizes your player in the game: your name, paddle color,
 * paddle speed and size, your own sounds, and the rules you'd like to play.
 *
 * NOTE: Custom settings need the 2026 version of the DF Pong game.
 * Older versions of the game ignore them, and your controller still
 * works there as a normal controller.
 *
 * HARDWARE SETUP:
 * ---------------
 * - Connect your UP button/switch between pin 2 and GND
 * - Connect your DOWN button/switch between pin 3 and GND
 * - No external resistors needed! We use INPUT_PULLUP mode.
 *
 * TWO KINDS OF SETTINGS:
 * ----------------------
 * 1. PLAYER settings only change YOUR side of the game:
 *      setPlayerName, setPlayerColor, setMoveSpeed, setPaddleSize,
 *      setHitSound, setScoreSound, setWinSound, setMusic
 *    The game uses them as long as the teacher (admin) allows them.
 *
 * 2. GAME settings are shared by both players, so you can only REQUEST them:
 *      requestPointsToWin, requestPuckSpeed, requestSpeedIncrement
 *    If both players ask for different values, the game combines the two
 *    requests (for example by averaging them) when the match starts, and
 *    shows the final rules on screen before the countdown.
 *
 * Values that are out of range are fixed for you, and a message is printed
 * in the Serial Monitor - so keep it open while you experiment!
 *
 * TIP: design your settings (and test your own sound files) with the
 * Settings Builder, then paste the code it gives you into setup():
 * https://digitalfuturesocadu.github.io/df-pong/game/builder/
 *
 * LED STATUS PATTERNS:
 * --------------------
 * - Slow blink (500ms): Disconnected, waiting for game to connect
 * - Fast blink (100ms): Connected, completing handshake
 * - Solid ON: Ready to play!
 *
 * DF PONG: https://github.com/DigitalFuturesOCADU/df-pong
 *
 * =============================================================================
 */

// Include the DFPongController library
#include <DFPongController.h>

// =============================================================================
// PIN CONFIGURATION
// =============================================================================

int upButtonPin = 2;     // Pin for the UP direction button
int downButtonPin = 3;   // Pin for the DOWN direction button

// =============================================================================
// CONTROLLER CONFIGURATION
// =============================================================================
// Set your unique controller number (1-242)
// Each player needs a different number!

int controllerNumber = 1;  // <-- CHANGE THIS TO YOUR ASSIGNED NUMBER!

// =============================================================================
// CONTROLLER SETUP
// =============================================================================

DFPongController controller;

// =============================================================================
// SETUP - Runs once when Arduino powers on or resets
// =============================================================================

void setup()
{
    Serial.begin(9600);
    delay(1000);  // Wait for Serial to initialize

    Serial.println("=================================");
    Serial.println("DFPongController - Custom Settings");
    Serial.println("=================================");

    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);

    controller.setControllerNumber(controllerNumber);
    controller.setStatusLED(LED_BUILTIN);

    // ==========================================================================
    // PLAYER SETTINGS - these only change YOUR side of the game
    // ==========================================================================

    // --------------------------------------------------------------------------
    // Your name in the game (up to 20 characters)
    // --------------------------------------------------------------------------
    // Without this, the game uses the name from the class list.

    controller.setPlayerName("Kate");

    // --------------------------------------------------------------------------
    // Your paddle color
    // --------------------------------------------------------------------------
    // Option A: red, green and blue amounts, each from 0 to 255

    controller.setPlayerColor(208, 139, 91);     // a warm orange

    // Option B: a web color code (search "color picker" to find codes)
    // controller.setPlayerColor("#d08b5b");

    // Very dark colors are brightened a little by the game so you can
    // still see your paddle on the dark background.

    // --------------------------------------------------------------------------
    // How fast your paddle moves: 1 (very slow) to 100 (very fast)
    // --------------------------------------------------------------------------
    // The normal speed is 10.

    controller.setMoveSpeed(14);

    // --------------------------------------------------------------------------
    // How big your paddle is: 50 (half size) to 200 (double size) percent
    // --------------------------------------------------------------------------
    // Normal is 100. A bigger paddle is easier to play but may be slower
    // to control precisely - try it!

    controller.setPaddleSize(120);

    // --------------------------------------------------------------------------
    // Sounds
    // --------------------------------------------------------------------------
    // Each sound can be one of the game's BUILT-IN sounds.
    //   Recordings: pingpong, claves, tick, bell, gong, bikebell, flourish,
    //               applause, pip, pipgo, quindar, clock
    //   Synthesized: blip, boop, pluck, click, laser, drum, chirp, zap,
    //                thud, wood, glass, coin
    //   The 2024 originals: classic, wall, ding
    // Built-in sounds ship with the game, so they always work.

    controller.setHitSound("pluck");     // when YOUR paddle hits the puck
    controller.setWinSound("gong");      // when YOU win the match

    // Background music for your matches: drift, tabletop, arcade or nightbus
    // (or a link to your own music file). If your opponent picks music too,
    // the game swaps between your picks after each point.
    controller.setMusic("nightbus");

    // ...or a LINK to your own sound file (mp3 or wav).
    //
    // Rules for sound links:
    //   - It must start with https://   (not http://)
    //   - It must be a direct link to the file itself (ends in .mp3 or .wav),
    //     not a page that shows the file. Google Drive / Dropbox "share"
    //     links do NOT work.
    //   - Keep the sound short (under 2 seconds is best) and the file
    //     smaller than 2 MB. It must load within 8 seconds.
    //   - The whole link can be at most 200 characters.
    //
    // Two free places to put your sound file:
    //
    //   1. A public GitHub repository. Upload the file, open it on GitHub,
    //      click "Raw", and copy that link. It looks like:
    //        https://raw.githubusercontent.com/YOUR-NAME/YOUR-REPO/main/score.mp3
    //
    //   2. GitHub Pages (Settings > Pages in your repository). The link
    //      looks like:
    //        https://YOUR-NAME.github.io/YOUR-REPO/score.mp3
    //
    // If a sound can't be loaded, the game plays the default sound instead
    // and shows the reason in the lobby.
    //
    // Uncomment and replace with YOUR link:
    // controller.setScoreSound("https://raw.githubusercontent.com/YOUR-NAME/YOUR-REPO/main/score.mp3");

    // ==========================================================================
    // GAME SETTING REQUESTS - shared by both players
    // ==========================================================================
    // These are REQUESTS. The game combines both players' requests when the
    // match starts (the teacher can also lock them).

    controller.requestPointsToWin(7);              // 1-21 points (default 10)

    // controller.requestPuckSpeed(6);             // 1-20 (default 5)
    // controller.requestSpeedIncrement(0.2);      // 0-1 (default 0.15)

    // --------------------------------------------------------------------------
    // See exactly what will be sent to the game
    // --------------------------------------------------------------------------
    Serial.print("My settings: ");
    Serial.println(controller.getSettingsJSON());

    // --------------------------------------------------------------------------
    // Initialize Bluetooth and start advertising
    // --------------------------------------------------------------------------
    // Settings can be set before OR after begin(). If you change them later,
    // while connected, the game picks up the new values automatically
    // (see the 07_LiveSettings example).

    if (controller.begin())
    {
        Serial.println("Bluetooth initialized successfully!");
        Serial.println("Waiting for game to connect...");
    }
    else
    {
        Serial.println("ERROR: Failed to initialize Bluetooth!");
        Serial.println("Check that your board supports BLE.");

        // Blink the LED rapidly to indicate an error
        while (true)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }

    Serial.println("---------------------------------");
}

// =============================================================================
// LOOP - Runs continuously after setup()
// =============================================================================

void loop()
{
    // Update the controller - REQUIRED every loop!
    // This also sends your settings to the game.
    controller.update();

    // Read the buttons (LOW = pressed, because of INPUT_PULLUP)
    int upButtonState = digitalRead(upButtonPin);
    int downButtonState = digitalRead(downButtonPin);

    // Send the matching control
    if (upButtonState == LOW)
    {
        controller.sendControl(UP);
    }
    else if (downButtonState == LOW)
    {
        controller.sendControl(DOWN);
    }
    else
    {
        controller.sendControl(NEUTRAL);
    }
}
