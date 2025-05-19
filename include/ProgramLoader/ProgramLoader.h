/**
 * @file program_loader.h
 * @brief Header file for the GTU-C312 program loader
 */

#ifndef PROGRAM_LOADER_H
#define PROGRAM_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../Cpu/Cpu.h"

/**
 * @brief Class for loading GTU-C312 programs into the CPU memory
 */
class ProgramLoader {
private:
    enum class ParsingState {
        NONE,
        DATA_SECTION,
        INSTRUCTION_SECTION
    };

    /**
     * @brief Parse data section from program file
     * 
     * @param lines Vector of program file lines
     * @param startLine Line index to start parsing from
     * @return std::pair<std::map<size_t, long>, size_t> Parsed data and next line index
     */
    std::pair<std::vector<long>, size_t> parseDataSection(
        const std::vector<std::string>& lines, 
        size_t startLine);

    /**
     * @brief Parse instruction section from program file
     *
     * @param lines Vector of program file lines
     * @param startLine Line index to start parsing from
     * @return std::map<size_t, Instruction> Parsed instructions
     */
    std::map<long, Instruction> parseInstructionSection(
        const std::vector<std::string>& lines, 
        size_t startLine);

public:
    /**
     * @brief Load a program file into the CPU
     * 
     * @param filename Path to the program file
     * @param cpu Reference to the CPU object
     * @throw std::runtime_error If the file cannot be opened or parsed
     */
    void loadProgram(const std::string& filename, CPU& cpu);
};

#endif // PROGRAM_LOADER_H