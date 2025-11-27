#!/bin/bash

# Script to run tests

cd "$(dirname "$0")/../tests" || exit 1

echo "Running tests..."
echo ""

if [ "$1" == "quick" ]; then
    echo "Running quick test..."
    python3 quick_test.py
elif [ "$1" == "individual" ]; then
    echo "Running individual test suites..."
    python3 test_individual.py
else
    echo "Running full test suite..."
    python3 test_client.py
fi

