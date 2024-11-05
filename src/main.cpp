#ifdef _WIN32
    // to je zbog InitWindow()  defined twice
    #define NOGDI
    #define NOUSER
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
#endif

#include <chrono>
#include <thread>
#include <iostream>

#include "raylib.h"
#include "chip8.h"

const int SCALE_FACTOR = 12;
const int WINDOW_WIDTH = 64 * SCALE_FACTOR;
const int WINDOW_HEIGHT = 32 * SCALE_FACTOR;
const float EMULATION_SPEED = 1.0f / 700.0f;  // ~700Hz

// CHIP-8 Keypad layout:
// 1 2 3 C
// 4 5 6 D
// 7 8 9 E
// A 0 B F
const int KEYMAP[16] = {
    KEY_X,    // 0
    KEY_ONE,  // 1
    KEY_TWO,  // 2
    KEY_THREE,// 3
    KEY_Q,    // 4
    KEY_W,    // 5
    KEY_E,    // 6
    KEY_A,    // 7
    KEY_S,    // 8
    KEY_D,    // 9
    KEY_Z,    // A
    KEY_C,    // B
    KEY_FOUR, // C
    KEY_R,    // D
    KEY_F,    // E
    KEY_V     // F
};

void initializeWindow() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8 Emulator");
    SetTargetFPS(60);
}

void handleInput(Chip8& chip8) {
    for (int i = 0; i < 16; i++) {
        chip8.setKey(i, IsKeyDown(KEYMAP[i]));
    }
}

void renderDisplay(const Chip8& chip8) {
    BeginDrawing();
    ClearBackground(BLACK);

    const auto& display = chip8.getDisplay();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float scaleX = static_cast<float>(screenWidth) / 64.0f;
    float scaleY = static_cast<float>(screenHeight) / 32.0f;

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[y * 64 + x]) {
                DrawRectangle(
                    static_cast<int>(x * scaleX),
                    static_cast<int>(y * scaleY),
                    static_cast<int>(scaleX),
                    static_cast<int>(scaleY),
                    WHITE
                );
            }
        }
    }

    EndDrawing();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;
    try {
        chip8.loadROM(argv[1]);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    initializeWindow();

    auto lastCycle = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose()) {
        handleInput(chip8);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastCycle).count();

        if (deltaTime >= EMULATION_SPEED) {
            chip8.emulateCycle();
            lastCycle = currentTime;
        }

        renderDisplay(chip8);
    }

    CloseWindow();
    return 0;
}