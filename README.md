# GTU-C312 CPU Simulator

This project implements a CPU simulator for the fictional GTU-C312 architecture as described in the instructions.txt file.

## Project Structure

```
.
├── bin/                      # Compiled binaries
├── include/                  # Header files
│   ├── Cpu/                  # CPU related headers
│   ├── InstructionParser/    # Instruction parsing headers
│   └── ProgramLoader/        # Program loading headers
├── obj/                      # Object files
├── src/                      # Source files
│   ├── Cpu/                  # CPU implementation
│   ├── InstructionParser/    # Instruction parser implementation
│   ├── ProgramLoader/        # Program loader implementation
│   └── main.cpp              # Main entry point
├── instructions.txt          # CPU architecture specifications
├── Makefile                  # Build configuration
├── README.md                 # This file
└── sample_program.txt        # Example program
```

## Building the Project

To build the project, run:

```bash
make
```

This will compile the source files and create an executable in the `bin` directory.

## Running the Simulator

To run the simulator with the sample program:

```bash
make run
```

Or run the executable directly:

```bash
./bin/cpu_simulator sample_program.txt
```

You can also create your own programs following the format in `sample_program.txt` and run them:

```bash
./bin/cpu_simulator path/to/your_program.txt
```

## GTU-C312 Architecture

- Memory is represented as an array of signed long integers
- First 1000 addresses are reserved for kernel mode
- Memory locations 0-3 have special purposes:
  - 0: Program Counter (PC)
  - 1: Stack Pointer (SP)
  - 2: System Call Result
  - 3: Number of executed instructions

## Instruction Set

The GTU-C312 instruction set includes:

- SET B A - Set memory location A to value B
- CPY A1 A2 - Copy value from address A1 to address A2
- CPYI A1 A2 - Indirect copy
- ADD A B - Add value B to memory location A
- ADDI A1 A2 - Add value at A2 to memory location A1
- SUBI A1 A2 - Subtract value at A2 from A1, store in A2
- JIF A C - Jump to address C if value at A is <= 0
- PUSH A - Push value at address A onto stack
- POP A - Pop value from stack into address A
- CALL C - Call subroutine at address C
- RET - Return from subroutine
- HLT - Halt CPU execution
- USER - Switch to user mode
- SYSCALL - System calls (PRN, HLT, YIELD)
