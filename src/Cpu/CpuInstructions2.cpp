/**
 * @file cpu_instructions2.cpp
 * @brief Implementation of additional GTU-C312 CPU instruction handlers
 */

#include "../../include/Cpu/Cpu.h"
#include <iostream>
#include <string>

// SUBI    A1  A2      Indirect Subtraction: Subtract the contents of memory address A2 from
//                     address A1, put the result in A2
void CPU::executeSubi(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("SUBI requires 2 operands");
    
    long    destAddr = operands[1];
    long    srcAddr  = operands[0];
    
    checkAddressAccess(destAddr);
    checkAddressAccess(srcAddr);
    
    memory[destAddr] = memory[srcAddr] - memory[destAddr];
    incrementPC();
}

// JIF     A   C       Set the CPU program counter with C if memory location A content is less
//                     than or equal to 0
void CPU::executeJif(const std::vector<long>& operands) {
    if (operands.size() != 2)
        throw CPUException("JIF requires 2 operands");
    
    long condAddr = operands[0];
    long jumpAddr = operands[1];
    
    checkAddressAccess(condAddr);
    
    if (memory[condAddr] <= 0) {
        setPC(jumpAddr);
    } else {
        incrementPC();
    }
}

// PUSH    A           Push memory A onto the stack. Stack grows downwards.
void CPU::executePush(const std::vector<long>& operands) {
    if (operands.size() != 1)
        throw CPUException("PUSH requires 1 operand");
    
    long address = operands[0];
    checkAddressAccess(address);
    
    // Decrement stack pointer (stack grows downward)
    setSP(getSP() - 1);
    long sp = getSP();
    checkAddressAccess(sp);
    
    memory[sp] = memory[address];
    incrementPC();
}

// POP     A           Pop value from stack into memory A.
void CPU::executePop(const std::vector<long>& operands) {
    if (operands.size() != 1)
        throw CPUException("POP requires 1 operand");
    
    long address = operands[0];
    checkAddressAccess(address);
    
    long sp = getSP();
    checkAddressAccess(sp);
    
    memory[address] = memory[sp];
    
    // Increment stack pointer
    setSP(sp + 1);
    incrementPC();
}

// CALL    C           Call subroutine at instruction C, push return address.
void CPU::executeCall(const std::vector<long>& operands) {
    if (operands.size() != 1)
        throw CPUException("CALL requires 1 operand");
    
    long jumpAddr = operands[0];
    
    // Push return address (PC + 1) onto stack
    setSP(getSP() - 1);
    long sp = getSP();
    checkAddressAccess(sp);
    
    memory[sp] = getPC() + 1;  // Store PC + 1 as the return address
    
    // Jump to subroutine
    setPC(jumpAddr);
}
// RET                 Return from subroutine by popping return address.
void CPU::executeRet(const std::vector<long>& operands) {
    if (!operands.empty())
        throw CPUException("RET requires 0 operands");
    
    // Pop return address from stack
    long sp = getSP();
    checkAddressAccess(sp);
    
    long returnAddr = memory[sp];
    setSP(sp + 1);
    
    // Jump to return address
    setPC(returnAddr);
}

void CPU::executeHlt(const std::vector<long>& operands) {
    if (!operands.empty())
        throw CPUException("HLT requires 0 operands");
    
    // Print final state of key memory locations for debugging
    std::cerr << "---- Final CPU state ----" << std::endl;
    std::cerr << "PC: " << getPC() << std::endl;
    std::cerr << "SP: " << getSP() << std::endl;
    std::cerr << "----------------------" << std::endl;
    
    halted = true;
}

void CPU::executeUser(const std::vector<long>& operands) {
    if (!operands.empty())
        throw CPUException("USER requires 0 operands");
    mode = CPUMode::USER;
    incrementPC();
}

void CPU::executeSyscall(const std::vector<long>& operands) {
    if (operands.empty())
        throw CPUException("SYSCALL requires at least 1 operand");
    CPUMode prevMode = mode;
    long syscallCode = operands[0];
    std::string syscallType = "";
    if (syscallCode == 1) syscallType = "PRN";
    else if (syscallCode == 2) syscallType = "HLT";
    else if (syscallCode == 3) syscallType = "YIELD";
    if (syscallType == "PRN" && operands.size() >= 2) {
        long address = operands[1];
        checkAddressAccess(address);
        mode = CPUMode::KERNEL;
        // Store the result in R2 register (memory location 2)
        memory[2] = memory[address];
        if (prnCallback)
            prnCallback(memory[address]);
        incrementPC();
    } else if (syscallType == "HLT") {
        mode = CPUMode::KERNEL;
        // Store the result in R2 register (memory location 2)
        memory[2] = 0; // HLT success code
        incrementPC();
    } else if (syscallType == "YIELD") {
        mode = CPUMode::KERNEL;
        // Store the result in R2 register (memory location 2)
        memory[2] = 1; // YIELD success code
        if (yieldCallback)
            yieldCallback();
        incrementPC();
    } else
        throw CPUException("Unknown or invalid SYSCALL: " + syscallType);
    mode = prevMode;
}