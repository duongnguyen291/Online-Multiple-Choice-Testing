#!/bin/bash

# Script to build and run the server

cd "$(dirname "$0")/../server" || exit 1

echo "Building server..."
make

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Starting server..."
echo "Press Ctrl+C to stop"
echo ""

./bin/server

