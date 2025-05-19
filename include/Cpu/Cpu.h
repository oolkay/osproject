
/**
 * @file cpu.h
 * @brief Header file for the GTU-C312 CPU simulator
 */

#ifndef CPU_H
#define CPU_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <stdexcept>

/**
 * @brief Enum representing CPU mode
 */
enum class CPUMode {
    KERNEL,
    USER
};

/**
 * @brief CPU exception class for error handling
 */
class CPUException : public std::runtime_error {
public:
    explicit CPUException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Instruction structure
 */
struct Instruction {
    std::string opcode;
    std::vector<long> operands;
};

/**
 * @brief GTU-C312 CPU simulator class
 */
class CPU {
private:
    // Memory and registers
    std::vector<long>      memory;
    std::unordered_map<long, Instruction> instructions;  // Store instructions with their addresses
    bool                   halted;
    CPUMode                mode;
    
    // Special memory addresses
    static const size_t PC_ADDR = 0;
    static const size_t SP_ADDR = 1;
    static const size_t SYSCALL_RESULT_ADDR = 2;
    static const size_t INSTR_COUNT_ADDR = 3;
    static const size_t USER_MODE_BOUNDARY = 1000;
    
    // System call callbacks
    std::function<void(long)> prnCallback;
    std::function<void()> yieldCallback;
    
    // Instruction execution table
    std::unordered_map<std::string, std::function<void(const std::vector<long>&)>> 
        instructionHandlers;
    
    // Initialize instruction handlers
    void initInstructionHandlers();
    
    // Helper methods
    void checkAddressAccess(long address) const;
    void incrementPC();
    long getPC() const;
    void setPC(long value);
    long getSP() const;
    void setSP(long value);
    void printMemoryContents() const;
    void printInstructionMap() const;

    // Instruction handlers
    void executeSet(const std::vector<long>& operands);
    void executeCpy(const std::vector<long>& operands);
    void executeCpyi(const std::vector<long>& operands);
    void executeAdd(const std::vector<long>& operands);
    void executeAddi(const std::vector<long>& operands);
    void executeSubi(const std::vector<long>& operands);
    void executeJif(const std::vector<long>& operands);
    void executePush(const std::vector<long>& operands);
    void executePop(const std::vector<long>& operands);
    void executeCall(const std::vector<long>& operands);
    void executeRet(const std::vector<long>& operands);
    void executeHlt(const std::vector<long>& operands);
    void executeUser(const std::vector<long>& operands);
    void executeSyscall(const std::vector<long>& operands);

public:
    /**
     * @brief Construct a new CPU object
     * 
     * @param memorySize Total memory size
     * @param prnCallback Callback for SYSCALL PRN
     * @param yieldCallback Callback for SYSCALL YIELD
     */
    CPU(size_t memorySize, 
        std::function<void(long)> prnCallback,
        std::function<void()> yieldCallback);
    
    /**
     * @brief Reset the CPU state
     */
    void reset();
    
    /**
     * @brief Load data into memory
     * 
     * @param address Starting memory address
     * @param data Data to load
     */
    void loadMemory(size_t address, const std::vector<long>& data);
    
    /**
     * @brief Load instructions into the CPU
     * 
     * @param newInstructions Vector of instructions to load
     */
    void loadInstructions(const std::vector<Instruction>& newInstructions);
    
    /**
     * @brief Execute the next instruction
     */
    void execute();
    
    /**
     * @brief Check if CPU is halted
     * 
     * @return true if halted, false otherwise
     */
    bool isHalted() const;
    
    /**
     * @brief Get current CPU mode
     * 
     * @return CPUMode enum value
     */
    CPUMode getMode() const;
    
    /**
     * @brief Get memory contents
     * 
     * @return Reference to the memory vector
     */
    const std::vector<long>& getMemory() const;

    /**
     * @brief Parse instruction from string
     * 
     * @param instructionStr Instruction string
     * @return Parsed instruction
     */
    static Instruction parseInstruction(const std::string& instructionStr);
};

#endif // CPU_H