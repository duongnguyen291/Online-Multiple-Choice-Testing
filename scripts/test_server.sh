#!/bin/bash

# Quick test script to verify server can start

cd "$(dirname "$0")/.." || exit 1

echo "Testing server startup..."
echo ""

# Check if server binary exists
if [ ! -f "server/bin/server" ]; then
    echo "Server binary not found. Building..."
    cd server
    make
    cd ..
fi

# Try to start server in background
echo "Starting server..."
cd server
timeout 3 ./bin/server > /tmp/server_test.log 2>&1 &
SERVER_PID=$!

sleep 1

# Check if server is running
if ps -p $SERVER_PID > /dev/null; then
    echo "✓ Server started successfully!"
    echo "  PID: $SERVER_PID"
    echo "  Log: /tmp/server_test.log"
    
    # Kill server
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    
    echo ""
    echo "Last few lines of server log:"
    tail -5 /tmp/server_test.log
    echo ""
    echo "Server test passed!"
    exit 0
else
    echo "✗ Server failed to start"
    echo ""
    echo "Server log:"
    cat /tmp/server_test.log
    exit 1
fi

