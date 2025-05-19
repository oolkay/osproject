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

#include <map>
#include <sstream>
#include <iostream>

// Helper function to trim whitespace from both ends of a string.
static std::string trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Helper to remove comments and trim the result.
static std::string cleanLine(const std::string &line) {
    size_t pos = line.find('#');
    return trim(pos != std::string::npos ? line.substr(0, pos) : line);
}

// Helper to extract an address and instruction string from a line.
static std::pair<long, std::string> parseInstructionLine(const std::string &line) {
    std::istringstream iss(line);
    long address = -1;
    iss >> address;
    std::string instr;
    std::getline(iss, instr);
    return {address, trim(instr)};
}

// Helper to map SYSCALL type strings to a numeric code.
static long getSyscallCode(const std::string &syscallType) {
    if (syscallType == "PRN")
        return 1;
    else if (syscallType == "HLT")
        return 2;
    else if (syscallType == "YIELD")
        return 3;
    return 0;
}

std::map<long, Instruction> ProgramLoader::parseInstructionSection(
    const std::vector<std::string>& lines, size_t startLine) {
    
    std::map<long, Instruction> instructions;
    InstructionParser parser;
    
    while (startLine < lines.size() && 
           lines[startLine].find("End Instruction Section") == std::string::npos) {
       
        std::string line = cleanLine(lines[startLine++]);
        if (line.empty())
            continue;
       
        auto [address, instrStr] = parseInstructionLine(line);
        try {
            ParsedInstruction parsed = parser.parse(instrStr);
            Instruction inst;
            inst.opcode = parsed.opcode;
            if (parsed.opcode == "SYSCALL") {
                long syscallCode = getSyscallCode(parsed.syscallType);
                std::vector<long> newOps = {syscallCode};
                newOps.insert(newOps.end(), parsed.operands.begin(), parsed.operands.end());
                inst.operands = newOps;
            } else {
                inst.operands = parsed.operands;
            }
            instructions[address] = inst;
        } catch (const std::exception &e) {
            std::cerr << "Warning: Skipping invalid instruction at address " 
                      << address << ": " << instrStr << " - " << e.what() << std::endl;
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
    std::map<long, Instruction> instructions;
    
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
