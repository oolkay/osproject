/**
 * @file instruction_parser.cpp
 * @brief Implementation of the GTU-C312 instruction parser
 */

#include "../../include/InstructionParser/InstructionParser.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

InstructionParser::InstructionParser() {
    // Initialize the opcode to operand count map
    opcodeOperandCount = {
        {"SET", 2},
        {"CPY", 2},
        {"CPYI", 2},
        {"ADD", 2},
        {"ADDI", 2},
        {"SUBI", 2},
        {"JIF", 2},
        {"PUSH", 1},
        {"POP", 1},
        {"CALL", 1},
        {"RET", 0},
        {"HLT", 0},
        {"USER", 0},
        {"SYSCALL", 1} // SYSCALL always has at least one operand (the syscall type)
    };
    
    // Initialize the syscall type to operand count map
    syscallOperandCount = {
        {"PRN", 1},   // PRN takes one operand (memory address)
        {"HLT", 0},   // HLT takes no operands
        {"YIELD", 0}  // YIELD takes no operands
    };
}

ParsedInstruction InstructionParser::parse(const std::string& instructionStr) const {
    ParsedInstruction instruction;
    std::istringstream iss(instructionStr);
    std::string token;
    
    // Parse opcode
    iss >> token;
    instruction.opcode = token;
    
    // Special handling for SYSCALL
    if (instruction.opcode == "SYSCALL") {
        // Parse syscall type
        iss >> token;
        instruction.syscallType = token;
        
        // Check if syscall type is valid
        if (syscallOperandCount.find(instruction.syscallType) == syscallOperandCount.end()) {
            throw std::runtime_error("Invalid syscall type: " + instruction.syscallType);
        }
    }
    
    // Delegate operand parsing to the helper function to keep this under 30 lines.
    parseOperands(iss, instruction);
    
    // Validate the instruction
    validateOperandCount(instruction);
    
    return instruction;
}

void InstructionParser::parseOperands(std::istringstream& iss, ParsedInstruction& instruction) const {
    std::string token;
    while (iss >> token) {
        // Skip comments
        if (token.front() == '#') {
            break;
        }
        
        try {
            instruction.operands.push_back(std::stol(token));
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Invalid operand: " + token);
        } catch (const std::out_of_range&) {
            throw std::runtime_error("Operand out of range: " + token);
        }
    }
}

void InstructionParser::validateOperandCount(
    const ParsedInstruction& instruction) const {
    
    // Check if opcode is valid
    auto opcodeIter = opcodeOperandCount.find(instruction.opcode);
    if (opcodeIter == opcodeOperandCount.end()) {
        throw std::runtime_error("Invalid opcode: " + instruction.opcode);
    }        // For SYSCALL, check syscall type operand count
        if (instruction.opcode == "SYSCALL") {
            auto syscallIter = syscallOperandCount.find(instruction.syscallType);
            if (syscallIter == syscallOperandCount.end()) {
                throw std::runtime_error("Invalid syscall type: " + instruction.syscallType);
            }
            
            int expectedCount = syscallIter->second;
            if (static_cast<int>(instruction.operands.size()) != expectedCount) {
                throw std::runtime_error("Syscall " + instruction.syscallType + 
                                        " requires " + std::to_string(expectedCount) + 
                                        " operands, got " + 
                                        std::to_string(instruction.operands.size()));
            }
        } else {
            // For regular instructions, check operand count
            int expectedCount = opcodeIter->second;
            if (static_cast<int>(instruction.operands.size()) != expectedCount) {
                throw std::runtime_error("Instruction " + instruction.opcode + 
                                        " requires " + std::to_string(expectedCount) + 
                                        " operands, got " + 
                                        std::to_string(instruction.operands.size()));
            }
    }
}