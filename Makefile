.PHONY: all clean build run server gateway client help

# Default target
all: build

# Build all components
build: server gateway
	@echo "✓ All components built successfully"

# Build server
server:
	@echo "Building server..."
	@cd server && make clean && make
	@echo "✓ Server built"

# Build gateway
gateway:
	@echo "Building gateway..."
	@cd gateway && make clean && make
	@echo "✓ Gateway built"

# Run all services (requires tmux)
run: build
	@echo "Starting all services..."
	@chmod +x run.sh
	@./run.sh

# Run with simple method (no tmux)
run-simple: build
	@echo "Starting all services..."
	@chmod +x run-simple.sh
	@./run-simple.sh

# Run only server
run-server:
	@cd server && make run

# Run only gateway
run-gateway:
	@cd gateway && make run

# Run only web client
run-client:
	@cd client && python3 -m http.server 3000

# Serve web client on different port
serve-client:
	@echo "Web client available at http://localhost:3000"
	@cd client && python3 -m http.server 3000

# Clean all build artifacts
clean:
	@echo "Cleaning all components..."
	@cd server && make clean
	@cd gateway && make clean
	@echo "✓ All cleaned"

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y build-essential libssl-dev
	@echo "✓ Dependencies installed"

# Setup database
setup-db:
	@echo "Setting up database..."
	@chmod +x scripts/setup_db.sh
	@./scripts/setup_db.sh
	@echo "✓ Database setup complete"

# Quick test (connect to running server)
quick-test:
	@cd tests && python3 quick_test.py

# Full test suite
test:
	@cd tests && python3 test_client.py

# Show help
help:
	@echo "Online Testing System - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build all components (default)"
	@echo "  build            - Build server and gateway"
	@echo "  server           - Build server only"
	@echo "  gateway          - Build gateway only"
	@echo ""
	@echo "Running:"
	@echo "  run              - Start all services (requires tmux)"
	@echo "  run-simple       - Start all services (no tmux required)"
	@echo "  run-server       - Start server only"
	@echo "  run-gateway      - Start gateway only"
	@echo "  run-client       - Start web client server"
	@echo ""
	@echo "Utility:"
	@echo "  clean            - Remove all build artifacts"
	@echo "  install-deps     - Install required dependencies"
	@echo "  setup-db         - Initialize database"
	@echo "  test             - Run test suite"
	@echo "  quick-test       - Quick connectivity test"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make run          # Start all services"
	@echo "  make clean        # Clean build"
	@echo "  make test         # Run tests"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make clean && make"
	@echo "  2. make run"
	@echo "  3. Open http://localhost:3000 in browser"
