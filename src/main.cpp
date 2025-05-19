/**
 * @file main.cpp
 * @brief Main entry point for the GTU-C312 CPU simulator
 * 
 * This program simulates the GTU-C312 CPU and runs GTU-C312 assembly programs.
 * It supports three debug modes:
 * - Debug Mode 0: Output memory contents after CPU halts
 * - Debug Mode 1: Output current instruction and memory after each instruction execution
 * - Debug Mode 2: Output current instruction and memory after each instruction and wait for keypress
 * 
 * Usage: ./cpu_simulator <program_file> [-D debug_mode]
 */

#include "../include/Cpu/Cpu.h"
#include "../include/ProgramLoader/ProgramLoader.h"
#include "../include/InstructionParser/InstructionParser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * Print callback function for SYSCALL PRN
 * The value is also stored in memory location 2 (system call result register)
 */
void printCallback(long value) {
    std::cout << "PRN: " << value << std::endl;
}

/**
 * Yield callback function for SYSCALL YIELD
 */
void yieldCallback() {
    std::cout << "YIELD called" << std::endl;
}

/**
 * Convert a ParsedInstruction to a CPU Instruction
 */
Instruction convertToCpuInstruction(const ParsedInstruction& parsed) {
    Instruction inst;
    inst.opcode = parsed.opcode;
    
    // Handle SYSCALL instructions with syscall type
    if (parsed.opcode == "SYSCALL") {
        // Map syscall type to a numeric code
        long syscallCode = 0;
        if (parsed.syscallType == "PRN") syscallCode = 1;
        else if (parsed.syscallType == "HLT") syscallCode = 2;
        else if (parsed.syscallType == "YIELD") syscallCode = 3;
        
        // Add syscall code as first operand
        std::vector<long> newOperands = {syscallCode};
        
        // Add remaining operands
        newOperands.insert(newOperands.end(), parsed.operands.begin(), 
                          parsed.operands.end());
        
        inst.operands = newOperands;
    } else {
        inst.operands = parsed.operands;
    }
    
    return inst;
}

// Function to print memory contents
void printMemoryContents(const std::vector<long>& memory) {
    std::cerr << "Memory Contents:" << std::endl;
    for (size_t i = 0; i < memory.size(); ++i) {
        if (memory[i] != 0) {  // Only print non-zero locations to reduce output size
            std::cerr << "Memory[" << i << "] = " << memory[i] << std::endl;
        }
    }
    std::cerr << "End of Memory Contents" << std::endl;
}

// Function to print current instruction info
void printInstructionInfo(const CPU& cpu, const std::unordered_map<long, std::string>& addressToInstructionStr, long currentPC) {
    std::cerr << "----------------------------------------" << std::endl;
    std::cerr << "Current PC: " << currentPC << std::endl;
    std::cerr << "Current Instruction: ";
    
    auto it = addressToInstructionStr.find(currentPC);
    if (it != addressToInstructionStr.end()) {
        std::cerr << it->second;
    } else {
        std::cerr << "Unknown (not in instruction map)";
    }
    std::cerr << std::endl;
    
    std::cerr << "CPU Mode: " << (cpu.getMode() == CPUMode::KERNEL ? "KERNEL" : "USER") << std::endl;
    std::cerr << "Instruction Count: " << cpu.getMemory()[3] << std::endl;
    std::cerr << "----------------------------------------" << std::endl;
}

std::unordered_map<long, std::string> loadInstructionMap(const std::string& filename) {
    std::unordered_map<long, std::string> addrMap;
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open program file: " + filename);
    std::string line;
    long address = 0;
    bool inSection = false;
    while (std::getline(file, line)) {
        // Remove comments
        size_t pos = line.find('#');
        if (pos != std::string::npos) line = line.substr(0, pos);
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;
        if (line.find("Begin Instruction Section") != std::string::npos) { inSection = true; continue; }
        if (line.find("End Instruction Section") != std::string::npos) { inSection = false; continue; }
        if (inSection) { addrMap[address++] = line; }
    }
    return addrMap;
}

void executeProgram(CPU& cpu, const std::unordered_map<long, std::string>& addrMap, int debugMode) {
    int steps = 0;
    while (!cpu.isHalted()) {
        long currentPC = cpu.getMemory()[0];
        if (debugMode > 0)
            printInstructionInfo(cpu, addrMap, currentPC);
        try {
            cpu.execute();
            steps++;
            if (debugMode == 1)
                printMemoryContents(cpu.getMemory());
            else if (debugMode == 2) {
                printMemoryContents(cpu.getMemory());
                std::cout << "Press Enter to continue..." << std::endl;
                std::cin.get();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error at step " << steps << ", PC=" << currentPC
                      << ": " << e.what() << std::endl;
            throw;
        }
    }
    if (debugMode == 0)
        printMemoryContents(cpu.getMemory());
    std::cout << "Program halted. Total instructions executed: " 
              << cpu.getMemory()[3] << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program_file> [-D debug_mode]" << std::endl;
        std::cerr << "Debug modes:" << std::endl;
        std::cerr << "  0: Output memory contents after CPU halts" << std::endl;
        std::cerr << "  1: Output current instruction and memory after each instruction execution" << std::endl;
        std::cerr << "  2: Output current instruction and memory after each instruction and wait for keypress" << std::endl;
        return 1;
    }

    try {
        // Create CPU with 11000 memory locations (as per the project requirements)
        // This allows for OS (0-999) and 10 threads (1000-10999)
        // Memory layout:
        // - 0: Program Counter (PC)
        // - 1: Stack Pointer (SP)
        // - 2: System call result register
        // - 3: Instruction count
        CPU cpu(11000, printCallback, yieldCallback);
        ProgramLoader loader;
        
        auto addrMap = loadInstructionMap(argv[1]);
        std::cout << "Loading program from " << argv[1] << "..." << std::endl;
        loader.loadProgram(argv[1], cpu);
        
        int debugMode = 0;
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-D" && i + 1 < argc) {
                debugMode = std::stoi(argv[++i]);
                std::cout << "Debug mode " << debugMode << " enabled" << std::endl;
            }
        }
        
        std::cout << "Executing program..." << std::endl;
        executeProgram(cpu, addrMap, debugMode);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
