# GTU-C312 CPU Simulator Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
INCLUDE = -I./include

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))
$(shell mkdir -p $(OBJ_DIR)/Cpu $(OBJ_DIR)/InstructionParser $(OBJ_DIR)/ProgramLoader)

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Cpu/Cpu.cpp \
          $(SRC_DIR)/Cpu/CpuInstructions.cpp \
          $(SRC_DIR)/Cpu/CpuInstructions2.cpp \
          $(SRC_DIR)/InstructionParser/InstructionParser.cpp \
          $(SRC_DIR)/ProgramLoader/ProgramLoader.cpp

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# Executable name
EXECUTABLE = $(BIN_DIR)/cpu_simulator

# Default target
all: $(EXECUTABLE)

# Link object files to create executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run the simulator with a sample program
run: $(EXECUTABLE)
	$(EXECUTABLE) sample_program.txt

.PHONY: all clean run
