#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <memory>
#include <iostream>
#include <vector>
#include <sstream>

typedef unsigned char (*function)(unsigned char);

const int memorySize = 4096;
const size_t changeIndex = 6;
void* globalMemory;

// Add 1 to argument
unsigned char program[] = {
        0x48, 0x89, 0xf8,
        0x48, 0x83, 0xc0,
        0x01, 0xc3
};

void* allocateMemory(size_t size) {
    void* pointer = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pointer == MAP_FAILED) {
        std::cerr << "Can't allocate memory" << std::endl;
        exit(EXIT_FAILURE);
    }
    return pointer;
}

void freeMemory(void* memory, size_t size) {
    if (munmap(memory, size) == -1) {
        std::cerr << "Can't free allocated memory" << std::endl;
        exit(EXIT_FAILURE);
    }
    return;
}

void switchMemoryProt(void* memory, size_t size, int prot) {
    if (mprotect(memory, size, PROT_READ | prot) == -1) {
        std::cerr << "Can't switch memory prot" << std::endl;
        exit(EXIT_FAILURE);
    }
    return;
}

void deployProgram(void* memory) {
    memcpy(memory, program, sizeof(program));
    return;
}

void changeProgram(void* memory, size_t size, unsigned char number, size_t position) {
    switchMemoryProt(memory, size, PROT_WRITE);
    ((unsigned char*)memory)[position] = number;
    return;
}

unsigned char executeProgram(void* memory, size_t size, unsigned char value) {
    auto func = (function)memory;
    switchMemoryProt(memory, size, PROT_EXEC);
    unsigned char result = func(value);
    switchMemoryProt(memory, size, PROT_WRITE);
    return result;
}

int main() {
    globalMemory = allocateMemory(memorySize);
    deployProgram(globalMemory);
    std::cout << "This program adds 1 to your number x by default." << std::endl
                << "To start execution type \"execute your_value\"" << std::endl
                << "To change second argument type \"change your_value\"" << std::endl
                << "To stop the program type \"exit\"" << std::endl
                << "Arguments and result are stored in unsigned char (from 0 to 255)" << std::endl;
    std::string line;
    while (getline(std::cin, line)) {
       std::istringstream stream(line);
       std::string command;
       int argument;
       stream >> command;
       if (command == "execute") {
           stream >> argument;
           unsigned char casted = static_cast<unsigned char>(argument);
           int result = executeProgram(globalMemory, memorySize, casted);
           std::cout << "Result: " << result << std::endl;
       }
       else if (command == "change") {
           stream >> argument;
           unsigned char casted = static_cast<unsigned char>(argument);
           changeProgram(globalMemory, memorySize, casted, changeIndex);
           std::cout << "Second argument changed" << std::endl;
       }
       else if (command == "exit") {
           freeMemory(globalMemory, memorySize);
           break;
       }
       else {
           std::cout << "Unknown command" << std::endl;
       }
    }
    return 0;
}