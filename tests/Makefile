# Makefile for Protocol Unit Tests

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I../server/include
LDFLAGS = -lsqlite3 -lssl -lcrypto -lpthread

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = bin

# Source files for unit test
UNIT_TEST_SRC = test_protocol_unit.cpp
UNIT_TEST_OBJ = $(BUILD_DIR)/test_protocol_unit.o

# Server object files needed for linking
SERVER_SRC_DIR = ../server/src
SERVER_OBJS = $(BUILD_DIR)/protocol.o $(BUILD_DIR)/logger.o

# Target
TARGET = $(BIN_DIR)/test_protocol_unit

.PHONY: all clean test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile server objects
$(BUILD_DIR)/protocol.o: $(SERVER_SRC_DIR)/protocol.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I../server/include -c $< -o $@

$(BUILD_DIR)/logger.o: $(SERVER_SRC_DIR)/logger.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I../server/include -c $< -o $@

# Compile unit test
$(UNIT_TEST_OBJ): $(UNIT_TEST_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link
$(TARGET): $(UNIT_TEST_OBJ) $(SERVER_OBJS) | $(BIN_DIR)
	$(CXX) $(UNIT_TEST_OBJ) $(SERVER_OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

test: $(TARGET)
	./$(TARGET)

help:
	@echo "Protocol Unit Test Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build unit test (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  test    - Build and run unit test"
	@echo "  help    - Show this help"

