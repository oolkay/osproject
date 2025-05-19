/**
 * @file Cpu.cpp
 * @brief Implementation of the GTU-C312 CPU simulator
 */

#include "../../include/Cpu/Cpu.h"
#include <iostream>
#include <sstream>
#include <algorithm>

CPU::CPU(size_t memorySize, 
         std::function<void(long)> prnCallback,
         std::function<void()> yieldCallback)
    : memory(memorySize, 0),
      halted(false),
      mode(CPUMode::KERNEL),
      prnCallback(std::move(prnCallback)),
      yieldCallback(std::move(yieldCallback)) {
    
    initInstructionHandlers();
}

void CPU::reset() {
    std::fill(memory.begin(), memory.end(), 0);
    instructions.clear();
    halted = false;
    mode = CPUMode::KERNEL;
}

void CPU::loadMemory(size_t address, const std::vector<long>& data) {
    if (address + data.size() > memory.size())
        throw CPUException("Memory out of bounds");
    std::copy(data.begin(), data.end(), memory.begin() + address);
}

void CPU::loadInstructions(const std::vector<Instruction>& newInstructions) {
    instructions.clear();
    for (size_t i = 0; i < newInstructions.size(); i++) {
        instructions[i] = newInstructions[i];
    }
}

void CPU::execute() {
    if (halted)
        return;
    
    // Get instruction at PC
    long currentPC = getPC();
    
    // Increment instruction count
    memory[INSTR_COUNT_ADDR]++;
    
    // Check if PC is within bounds
    if (currentPC < 0 || currentPC >= static_cast<long>(memory.size()))
        throw CPUException("Program counter out of bounds: " + std::to_string(currentPC));
    
    // Get the current instruction from the instruction memory
    auto instrIt = instructions.find(currentPC);
    if (instrIt == instructions.end()) {
        throw CPUException("Instruction not found at address: " + std::to_string(currentPC));
    }
    
    // Get the instruction
    const Instruction& instruction = instrIt->second;
    std::string opcode = instruction.opcode;
    std::vector<long> operands = instruction.operands;
    

    // Find instruction handler and execute
    auto handler = instructionHandlers.find(opcode);
    if (handler != instructionHandlers.end()) {
        handler->second(operands);
    } else {
        throw CPUException("Unknown instruction: " + opcode);
    }
}

bool CPU::isHalted() const {
    return halted;
}

CPUMode CPU::getMode() const {
    return mode;
}

const std::vector<long>& CPU::getMemory() const {
    return memory;
}

void CPU::checkAddressAccess(long address) const {
    if (address < 0 || address >= static_cast<long>(memory.size()))
        throw CPUException("Memory access out of bounds: " + std::to_string(address));
    
    if (mode == CPUMode::USER && address < static_cast<long>(USER_MODE_BOUNDARY))
        throw CPUException("Access violation: User mode cannot access address " + 
                          std::to_string(address));
}

void CPU::incrementPC() {
    memory[PC_ADDR]++;
}

long CPU::getPC() const {
    return memory[PC_ADDR];
}

void CPU::setPC(long value) {
    memory[PC_ADDR] = value;
}

long CPU::getSP() const {
    return memory[SP_ADDR];
}

void CPU::setSP(long value) {
    memory[SP_ADDR] = value;
}

void CPU::printInstructionMap() const
{
    std::cerr << "Instruction Map:" << std::endl;
    for (const auto& pair : instructions) {
        std::cerr << "Address: " << pair.first << ", Instruction: " 
                  << pair.second.opcode << " ";
        for (const auto& operand : pair.second.operands) {
            std::cerr << operand << " ";
        }
        std::cerr << std::endl;
    }
}


Instruction CPU::parseInstruction(const std::string &instructionStr)
{
    Instruction inst;
    std::istringstream iss(instructionStr);
    
    // Parse opcode
    iss >> inst.opcode;
    
    // Parse operands
    long operand;
    while (iss >> operand) {
        inst.operands.push_back(operand);
    }
    
    return inst;
}