#ifndef CHIP8
#define CHIP8

#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <cstdint>

class Chip8 {



private:
    static const int MEMORY_SIZE = 4096;
    static const int PROGRAM_START = 0x200;
    static const int DISPLAY_WIDTH = 64;
    static const int DISPLAY_HEIGHT = 32;
    static const int FONTSET_START_ADDRESS = 0x50;

    // System components
    std::vector<uint8_t> memory;
    std::vector<uint8_t> V;  // Registers
    uint16_t I;              // Index register
    uint16_t pc;             // Program counter
    std::stack<uint16_t> stack;
    uint8_t delayTimer;
    uint8_t soundTimer;
    std::vector<bool> display;
    std::vector<bool> keypad;
    std::mt19937 rng;        // Random number generator
    bool drawFlag;

    // Font set
    static const std::vector<uint8_t> fontset;

    // Private methods
    uint16_t fetchOpcode();
    void executeOpcode(uint16_t opcode);
    void handleZeroOp(uint16_t opcode);
    void handleArithmetic(uint16_t opcode, uint8_t x, uint8_t y, uint8_t n);
    void handleMiscOps(uint16_t opcode, uint8_t x, uint8_t nn);
    void drawSprite(uint8_t x, uint8_t y, uint8_t n);
    void updateTimers();
    uint8_t readMemory(uint16_t address);
    void writeMemory(uint16_t address, uint8_t value);

public:
    Chip8() : 
        memory(MEMORY_SIZE),
        V(16),
        display(DISPLAY_WIDTH * DISPLAY_HEIGHT),
        keypad(16),
        I(0),
        pc(PROGRAM_START),
        delayTimer(0),
        soundTimer(0),
        drawFlag(false),
        rng(std::random_device{}()) {
        initialize();
    }
    void initialize();
    void loadROM(const std::string& filename);
    void emulateCycle();
    const std::vector<bool>& getDisplay() const { return display; }
    void setKey(int key, bool pressed);
    bool getDrawFlag() const { return drawFlag; }
    void setDrawFlag(bool value) { drawFlag = value; }
};
#endif