#!/bin/bash

# Online Testing System - Complete Startup Script
# This script starts:
# 1. Backend TCP Server (port 8888)
# 2. WebSocket Gateway (port 8080)
# 3. Web Server for Client (port 3000)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "============================================"
echo "Online Testing System - Startup Script"
echo "============================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if tmux is installed
if ! command -v tmux &> /dev/null; then
    echo -e "${RED}Error: tmux is not installed${NC}"
    echo "Install tmux: sudo apt-get install tmux"
    exit 1
fi

# Create or use existing tmux session
SESSION="online-testing"

# Kill existing session if it exists
tmux kill-session -t $SESSION 2>/dev/null || true

# Create new session
tmux new-session -d -s $SESSION

# Window 0: Backend Server
echo -e "${YELLOW}Starting Backend Server on port 8888...${NC}"
tmux send-keys -t $SESSION "cd '$SCRIPT_DIR/server' && make clean && make run" Enter
sleep 3

# Window 1: WebSocket Gateway
echo -e "${YELLOW}Starting WebSocket Gateway on port 8080...${NC}"
tmux new-window -t $SESSION
tmux send-keys -t $SESSION "cd '$SCRIPT_DIR/gateway' && make clean && make run" Enter
sleep 3

# Window 2: Web Server
echo -e "${YELLOW}Starting Web Server on port 3000...${NC}"
tmux new-window -t $SESSION
tmux send-keys -t $SESSION "cd '$SCRIPT_DIR/client' && python3 -m http.server 3000" Enter
sleep 2

# Window 3: Logs
tmux new-window -t $SESSION
tmux send-keys -t $SESSION "echo 'All services started!' && echo '' && echo 'Web Client: http://localhost:3000' && echo 'Backend: localhost:8888' && echo 'Gateway: localhost:8080' && echo '' && echo 'To access logs, use: tmux capture-pane -t online-testing -p'" Enter

# Display info
echo ""
echo -e "${GREEN}✓ All services started!${NC}"
echo ""
echo "======= Access Points ======="
echo "Web Client:      http://localhost:3000"
echo "Backend Server:  localhost:8888 (TCP)"
echo "WebSocket Gateway: localhost:8080 (WebSocket)"
echo ""
echo "======= Tmux Commands ======="
echo "View all windows:     tmux list-windows -t $SESSION"
echo "Attach to session:    tmux attach-session -t $SESSION"
echo "View backend logs:    tmux capture-pane -t $SESSION:0 -p"
echo "View gateway logs:    tmux capture-pane -t $SESSION:1 -p"
echo "Kill all:             tmux kill-session -t $SESSION"
echo ""
echo "Open browser and go to http://localhost:3000"
echo ""
echo -e "${YELLOW}Press Ctrl+C to exit this script (services will keep running)${NC}"
echo ""

# Keep script running
tail -f /dev/null
