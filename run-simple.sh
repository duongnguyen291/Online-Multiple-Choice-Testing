#!/bin/bash

# Simple startup script using separate terminals
# Prerequisites: xterm or gnome-terminal installed

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Starting Online Testing System..."
echo ""

# Function to start service in new terminal
start_service() {
    local service=$1
    local port=$2
    local command=$3
    
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal -- bash -c "cd '$SCRIPT_DIR/$service' && $command; bash"
    elif command -v xterm &> /dev/null; then
        xterm -e "cd '$SCRIPT_DIR/$service' && $command" &
    elif command -v xfce4-terminal &> /dev/null; then
        xfce4-terminal -e "cd '$SCRIPT_DIR/$service' && $command" &
    else
        echo "No terminal emulator found. Running in background..."
        cd "$SCRIPT_DIR/$service"
        eval "$command" &
    fi
}

# Start services
start_service "server" "8888" "make run"
sleep 2

start_service "gateway" "8080" "make run"
sleep 2

start_service "client" "3000" "python3 -m http.server 3000"

echo ""
echo "========================================="
echo "Services started!"
echo "========================================="
echo "Backend:   localhost:8888"
echo "Gateway:   localhost:8080"
echo "Web UI:    http://localhost:3000"
echo ""
echo "Open http://localhost:3000 in your browser"
echo "=========================================\n"

wait
