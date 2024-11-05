#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include "chip8.h"

int main() {
    // Your test code here
    std::cout << "Running CHIP-8 tests..." << std::endl;
    // Example test
    Chip8 chip8;
    chip8.initialize();
    // Add more tests as needed
    return 0;
}