# Makefile for Online Testing Server

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I./include
LDFLAGS = -lsqlite3 -lssl -lcrypto -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/server

# json library (nlohmann/json - header only)
JSON_HEADER = include/nlohmann/json.hpp

.PHONY: all clean run install_deps

all: $(TARGET)

# Create directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Download json library if not exists
$(JSON_HEADER):
	@echo "Downloading nlohmann/json library..."
	mkdir -p include/nlohmann
	wget -q https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -O $(JSON_HEADER)
	@echo "JSON library downloaded successfully"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR) $(JSON_HEADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Run server
run: $(TARGET)
	./$(TARGET)

# Install dependencies (for reference, requires sudo)
install_deps:
	@echo "Installing dependencies..."
	@echo "Run: sudo apt-get install -y g++ make sqlite3 libsqlite3-dev libssl-dev"
	
# Help
help:
	@echo "Online Testing Server - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the server (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  run          - Build and run the server"
	@echo "  install_deps - Show command to install dependencies"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make         - Build the server"
	@echo "  make run     - Run the server"
	@echo "  make clean   - Clean build files"

