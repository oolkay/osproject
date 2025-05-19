/**
 * @file ProgramLoader.cpp
 * @brief Implementation of the GTU-C312 program loader
 */
#include "../../include/ProgramLoader/ProgramLoader.h"
#include "../../include/InstructionParser/InstructionParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

std::pair<std::vector<long>, size_t> ProgramLoader::parseDataSection(
    const std::vector<std::string>& lines, 
    size_t startLine) {
    std::vector<long> data(256, 0); // Pre-allocate memory with zeros (assuming 256 is enough)
    size_t lineIndex = startLine;
    
    // Process each line until "End Data Section" is found
    while (lineIndex < lines.size()) {
        std::string line = lines[lineIndex++];
        // Remove comments (everything after #)
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
            continue;
        // Check for end of section
        if (line.find("End Data Section") != std::string::npos)
            break;
        // Parse data as address-value pairs
        std::istringstream iss(line);
        long address, value;
        if (iss >> address >> value) {
            // Ensure address is valid
            if (address >= 0 && static_cast<size_t>(address) < data.size()) {
                data[address] = value;
            } else if (address >= 0 && static_cast<size_t>(address) >= data.size()) {
                // Resize if needed
                data.resize(static_cast<size_t>(address) + 1, 0);
                data[address] = value;
            }
        }
    }
    
    return {data, lineIndex};
}

std::vector<Instruction> ProgramLoader::parseInstructionSection(
    const std::vector<std::string>& lines, 
    size_t startLine) {
    
    std::vector<Instruction> instructions;
    InstructionParser parser;
    size_t lineIndex = startLine;
    long maxAddress = -1;  // Track highest instruction address
    
    // Process each line until "End Instruction Section" is found
    while (lineIndex < lines.size()) {
        std::string line = lines[lineIndex++];
        
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Check for end of section
        if (line.find("End Instruction Section") != std::string::npos) {
            break;
        }
        
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Skip if line is empty after removing comments
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        
        // Parse instruction address and instruction
        std::istringstream iss(line);
        long address;
        std::string instr;
        
        // Read the address
        if (iss >> address) {
            // Read the rest of the line as the instruction
            std::getline(iss, instr);
            
            // Skip if instruction is empty
            if (instr.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }
            
            // Update max address
            if (address > maxAddress) {
                maxAddress = address;
                // Make sure we have space in the vector
                if (static_cast<size_t>(address) >= instructions.size()) {
                    instructions.resize(address + 1);
                }
            }
            
            // Parse the instruction
            try {
                ParsedInstruction parsed = parser.parse(instr);
                
                // Convert to CPU Instruction
                instructions[address].opcode = parsed.opcode;
                    
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
                    
                    instructions[address].operands = newOperands;
                } else {
                    instructions[address].operands = parsed.operands;
                }
            } catch (const std::exception& e) {
                std::cerr << "Warning: Skipping invalid instruction at address " << address 
                          << ": " << instr << " - " << e.what() << std::endl;
            }
        }
    }
    
    return instructions;
}

void ProgramLoader::loadProgram(const std::string& filename, CPU& cpu) {
    // Open the file
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename);
    // Read all lines
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    // Parse file
    size_t lineIndex = 0;
    std::vector<long> data;
    std::vector<Instruction> instructions;
    
    while (lineIndex < lines.size()) {
        line = lines[lineIndex++];
        // Check for section markers
        if (line.find("Begin Data Section") != std::string::npos) {
            auto result = parseDataSection(lines, lineIndex);
            data = result.first;
            lineIndex = result.second;
        } else if (line.find("Begin Instruction Section") != std::string::npos)
            instructions = parseInstructionSection(lines, lineIndex);
    }
    // Load data and instructions into CPU
    if (!data.empty())
        cpu.loadMemory(0, data);
    if (!instructions.empty())
        cpu.loadInstructions(instructions);
    else
        throw std::runtime_error("No instructions found in file");
}
