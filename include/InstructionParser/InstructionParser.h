/**
 * @file instruction_parser.h
 * @brief Header file for the GTU-C312 instruction parser
 */

#ifndef INSTRUCTION_PARSER_H
#define INSTRUCTION_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * @brief Struct representing a GTU-C312 instruction
 */
struct ParsedInstruction {
    std::string opcode;
    std::string syscallType;  // Used for SYSCALL instructions
    std::vector<long> operands;
};

/**
 * @brief Class for parsing GTU-C312 instructions
 */
class InstructionParser {
private:
    // Map of instruction opcodes to their required number of operands
    std::unordered_map<std::string, int> opcodeOperandCount;
    
    // Map of syscall types to their required number of operands
    std::unordered_map<std::string, int> syscallOperandCount;

public:
    /**
     * @brief Construct a new Instruction Parser object
     */
    InstructionParser();
    
    /**
     * @brief Parse an instruction string into a ParsedInstruction object
     * 
     * @param instructionStr The instruction string to parse
     * @return ParsedInstruction The parsed instruction
     * @throw std::runtime_error If the instruction is invalid
     */
    ParsedInstruction parse(const std::string& instructionStr) const;
    
    /**
     * @brief Validate that an instruction has the correct number of operands
     * 
     * @param instruction The instruction to validate
     * @throw std::runtime_error If the instruction has an invalid number of operands
     */
    void validateOperandCount(const ParsedInstruction& instruction) const;

    /**
     * @brief Parse operands from the instruction string
     * 
     * @param iss Input string stream containing the instruction string
     * @param instruction The ParsedInstruction object to populate
     */
    void parseOperands(std::istringstream& iss, ParsedInstruction& instruction) const;


};



#endif // INSTRUCTION_PARSER_H