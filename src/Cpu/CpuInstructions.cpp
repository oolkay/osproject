/**
 * @file cpu_instructions.cpp
 * @brief Implementation of GTU-C312 CPU instruction handlers
 */

#include "../../include/Cpu/Cpu.h"
#include <iostream>


void CPU::initInstructionHandlers() {
    instructionHandlers["SET"] = [this](const std::vector<long>& operands) {
        executeSet(operands);
    };
    instructionHandlers["CPY"] = [this](const std::vector<long>& operands) {
        executeCpy(operands);
    };
    instructionHandlers["CPYI"] = [this](const std::vector<long>& operands) {
        executeCpyi(operands);
    };
    instructionHandlers["ADD"] = [this](const std::vector<long>& operands) {
        executeAdd(operands);
    };
    instructionHandlers["ADDI"] = [this](const std::vector<long>& operands) {
        executeAddi(operands);
    };
    instructionHandlers["SUBI"] = [this](const std::vector<long>& operands) {
        executeSubi(operands);
    };
    instructionHandlers["JIF"] = [this](const std::vector<long>& operands) {
        executeJif(operands);
    };
    instructionHandlers["PUSH"] = [this](const std::vector<long>& operands) {
        executePush(operands);
    };
    instructionHandlers["POP"] = [this](const std::vector<long>& operands) {
        executePop(operands);
    };
    instructionHandlers["CALL"] = [this](const std::vector<long>& operands) {
        executeCall(operands);
    };
    instructionHandlers["RET"] = [this](const std::vector<long>& operands) {
        executeRet(operands);
    };
    instructionHandlers["HLT"] = [this](const std::vector<long>& operands) {
        executeHlt(operands);
    };
    instructionHandlers["USER"] = [this](const std::vector<long>& operands) {
        executeUser(operands);
    };
    instructionHandlers["SYSCALL"] = [this](const std::vector<long>& operands) {
        executeSyscall(operands);
    };
}

// SET B A
// Direct Set: Set Ath memory location with number B.
// Example: SET 20, 100 writes the value of 20 to memory location 100.
// A: MEM LOC, B: UNSIGNED LONG
void CPU::executeSet(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("SET requires 2 operands");
    
    unsigned long value = operands[0];
    long address = operands[1];
    
    
    checkAddressAccess(address);
    memory[address] = value;
    
    // Special case for setting PC (don't increment PC if we're setting it directly)
    if (address != PC_ADDR)
        incrementPC();
}

// CPY A1 A2 
// Direct Copy: Copy the content of memory location A1 to memory A2.
// Example: CPY 100, 120 copies the memory value of address 100 to the
//          memory address 120
void CPU::executeCpy(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("CPY requires 2 operands");
    
    long srcAddr = operands[0];
    long destAddr = operands[1];
    
    checkAddressAccess(srcAddr);
    checkAddressAccess(destAddr);
    
    memory[destAddr] = memory[srcAddr];
    if (destAddr != PC_ADDR)
        incrementPC();
}

//CPYI    A1  A2      Indirect Copy: Copy the memory address indexed by A1 to memory
//                    address A2. Example: CPYI 100, 120: if memory address 100 contains
//                    200, then this instruction copies the contents of memory address 200 to
//                    memory location 120.
void CPU::executeCpyi(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("CPYI requires 2 operands");
    
    long srcIndirectAddr = operands[0];
    long destAddr = operands[1];
    
    checkAddressAccess(srcIndirectAddr);
    long srcAddr = memory[srcIndirectAddr];
    checkAddressAccess(srcAddr);
    checkAddressAccess(destAddr);
    
    memory[destAddr] = memory[srcAddr];
    if (destAddr != PC_ADDR)
        incrementPC();
}

// ADD     A   B       Add number B to memory location A
void CPU::executeAdd(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("ADD requires 2 operands");
    
    long address = operands[0];
    long value = operands[1];
    
    checkAddressAccess(address);
    memory[address] += value;
    incrementPC();
}

// ADDI    A1  A2      Indirect Add: Add the contents of memory address A2 to address A1.
void CPU::executeAddi(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("ADDI requires 2 operands");
    
    long destAddr = operands[0];
    long srcAddr = operands[1];
    
    checkAddressAccess(destAddr);
    checkAddressAccess(srcAddr);
    
    memory[destAddr] += memory[srcAddr];
    incrementPC();
}