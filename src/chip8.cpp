#include <fstream>
#include <stdexcept>

#include "chip8.h"

// Initialize static font set member
const std::vector<uint8_t> Chip8::fontset = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1 
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void Chip8::initialize() {
    std::fill(memory.begin(), memory.end(), 0);
    std::fill(V.begin(), V.end(), 0);
    std::fill(display.begin(), display.end(), false);
    std::fill(keypad.begin(), keypad.end(), false);
    
    I = 0;
    pc = PROGRAM_START;
    while(!stack.empty()) stack.pop();
    delayTimer = 0;
    soundTimer = 0;
    
    // Load fontset
    for(size_t i = 0; i < fontset.size(); i++) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void Chip8::loadROM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM file");
    }

    std::streamsize size = file.tellg();
    if (size > (MEMORY_SIZE - PROGRAM_START)) {
        throw std::runtime_error("ROM size exceeds available memory");
    }

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&memory[PROGRAM_START]), size);
}

void Chip8::emulateCycle() {
    uint16_t opcode = fetchOpcode();
    executeOpcode(opcode);
    updateTimers();

    if (drawFlag) {
        drawFlag = false;
    }
}

uint16_t Chip8::fetchOpcode() {
    return (memory[pc] << 8) | memory[pc + 1];
}

void Chip8::executeOpcode(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t nn = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000)
    {
        case 0x0000:
            handleZeroOp(opcode);
            break;
        case 0x1000:
            pc = nnn;
            break;
        case 0x2000:
            stack.push(pc);
            pc = nnn;
            break;
        case 0x3000:
            pc += (V[x] == nn ? 4 : 2);
            break;
        case 0x4000:
            pc += (V[x] != nn ? 4 : 2);
            break;
        case 0x5000:
            pc += (V[x] == V[y] ? 4 : 2);
            break;
        case 0x6000:
            V[x] = nn;
            pc += 2;
            break;
        case 0x7000:
            V[x] += nn;
            pc += 2;
            break;
        case 0x8000:
            handleArithmetic(opcode, x, y, n);
            break;
        case 0x9000:
            pc += (V[x] != V[y] ? 4 : 2);
            break;
        case 0xA000:
            I = nnn;
            pc += 2;
            break;
        case 0xB000: 
            pc = V[0] + nnn;
            break;
        case 0xC000:
            V[x] = rng() & nn;
            pc += 2;
            break;
        case 0xD000:
            drawSprite(x, y, n);
            pc += 2;
            break;
        case 0xE000:
            if (nn == 0x9E) {
                pc += (keypad[V[x]] ? 4 : 2);
            } else if (nn == 0xA1) {
                pc += (!keypad[V[x]] ? 4 : 2);
            } else {
                std::cerr << "Unsupported opcode: " << std::hex << opcode << std::endl;
                pc += 2;
            }
            break;
        case 0xF000:
            handleMiscOps(opcode, x, nn);
            break;
        default:
            std::cerr << "Unsupported opcode: " << std::hex << opcode << std::endl;
            pc += 2;
            break;
    }
}

void Chip8::updateTimers() {
    if (delayTimer > 0) delayTimer--;
    if (soundTimer > 0) soundTimer--;
}

void Chip8::setKey(int key, bool pressed) {
    if (key >= 0 && key < 16) {
        keypad[key] = pressed;
    }
}

uint8_t Chip8::readMemory(uint16_t address) {
    return memory[address];
}

void Chip8::writeMemory(uint16_t address, uint8_t value) {
    memory[address] = value;
}

void Chip8::handleZeroOp(uint16_t opcode) {
    if (opcode == 0x00E0) {
        std::fill(display.begin(), display.end(), false);
        drawFlag = true;
        pc += 2;
    }
    else if (opcode == 0x00EE) {
        pc = stack.top();
        stack.pop();
        pc += 2;
    }
    else {
        std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
        pc += 2;
    }
}

void Chip8::handleArithmetic(uint16_t opcode, uint8_t x, uint8_t y, uint8_t n) {
    switch (n) {
        case 0x0:
            V[x] = V[y];
            break;
        case 0x1:
            V[x] |= V[y];
            break;
        case 0x2:
            V[x] &= V[y];
            break;
        case 0x3:
            V[x] ^= V[y];
            break;
        case 0x4: {
            uint16_t sum = V[x] + V[y];
            V[0xF] = (sum > 255 ? 1 : 0);
            V[x] = static_cast<uint8_t>(sum);
            break;
        }
        case 0x5:
            V[0xF] = (V[x] > V[y] ? 1 : 0);
            V[x] -= V[y];
            break;
        case 0x6:
            V[0xF] = V[x] & 0x1;
            V[x] >>= 1;
            break;
        case 0x7:
            V[0xF] = (V[y] > V[x] ? 1 : 0);
            V[x] = V[y] - V[x];
            break;
        case 0xE:
            V[0xF] = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            break;
        default:
            std::cerr << "Unsupported arithmetic opcode: 0x" << std::hex << opcode << std::endl;
            break;
    }
    pc += 2;
}

void Chip8::handleMiscOps(uint16_t opcode, uint8_t x, uint8_t nn) {
    switch (nn) {
        case 0x07:
            V[x] = delayTimer;
            break;
        case 0x0A: {
            bool keyPressDetected = false;
            for (size_t i = 0; i < keypad.size(); i++) {
                if (keypad[i]) {
                    V[x] = static_cast<uint8_t>(i);
                    keyPressDetected = true;
                    break;
                }
            }
            if (!keyPressDetected)
                return; // Skip the cycle and wait for a key press
            break;
        }
        case 0x15:
            delayTimer = V[x];
            break;
        case 0x18:
            soundTimer = V[x];
            break;
        case 0x1E:
            I += V[x];
            break;
        case 0x29:
            I = static_cast<uint16_t>(FONTSET_START_ADDRESS + V[x] * 5);
            break;
        case 0x33:
            memory[I] = V[x] / 100;
            memory[I + 1] = (V[x] % 100) / 10;
            memory[I + 2] = V[x] % 10;
            break;
        case 0x55:
            for (int i = 0; i <= x; i++) {
                writeMemory(static_cast<uint16_t>(I + i), V[i]);
            }
            break;
        case 0x65:
            for (int i = 0; i <= x; i++) {
                V[i] = readMemory(static_cast<uint16_t>(I + i));
            }
            break;
        default:
            std::cerr << "Unsupported FX opcode: 0x" << std::hex << opcode << std::endl;
            break;
    }
    pc += 2;
}

void Chip8::drawSprite(uint8_t x, uint8_t y, uint8_t n) {
    V[0xF] = 0;
    for (int yLine = 0; yLine < n; yLine++) {
        uint8_t pixel = readMemory(static_cast<uint16_t>(I + yLine));
        for (int xLine = 0; xLine < 8; xLine++) {
            if ((pixel & (0x80 >> xLine)) != 0) {
                int xCoord = (V[x] + xLine) % DISPLAY_WIDTH;
                int yCoord = (V[y] + yLine) % DISPLAY_HEIGHT;
                
                // Convert 2D coordinates to 1D index
                int index = yCoord * DISPLAY_WIDTH + xCoord;
                
                if (display[index]) {
                    V[0xF] = 1;
                }
                
                display[index] = !display[index];
            }
        }
    }
    drawFlag = true;
}