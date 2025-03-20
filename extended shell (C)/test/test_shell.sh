#!/bin/bash

# Variables
SHELL_EXEC="./myshell"
TEST_OUTPUT="test_results.txt"
EXPECTED_OUTPUT="expected_results.txt"

# Helper function for running commands and capturing output
run_test() {
    local cmd="$1"
    echo "Running: $cmd" | tee -a $TEST_OUTPUT
    echo "$cmd" | $SHELL_EXEC >> $TEST_OUTPUT 2>&1
    echo "----------------------------------------" >> $TEST_OUTPUT
}

# Prepare test output
echo "Testing Shell Implementation" > $TEST_OUTPUT
echo "=============================" >> $TEST_OUTPUT

# Tests
echo "1. Basic Commands" | tee -a $TEST_OUTPUT
run_test "ls"
run_test "pwd"
run_test "echo Hello, World!"

echo "2. Change Directory (cd)" | tee -a $TEST_OUTPUT
run_test "cd /tmp"
run_test "pwd"
run_test "cd .."
run_test "pwd"
run_test "cd nonexistentdir"

echo "3. Input/Output Redirection" | tee -a $TEST_OUTPUT
run_test "cat < /etc/passwd"
run_test "ls > test_output.txt"
run_test "cat test_output.txt"
run_test "grep root < /etc/passwd > root_users.txt"
run_test "cat root_users.txt"

echo "4. Piping" | tee -a $TEST_OUTPUT
run_test "ls | grep c"
run_test "cat /etc/passwd | grep root"
run_test "ls > out.txt | cat"

echo "5. Background Execution" | tee -a $TEST_OUTPUT
run_test "sleep 5 &"
run_test "jobs"

echo "6. Signal Handling" | tee -a $TEST_OUTPUT
run_test "sleep 30 &"
run_test "stop 1" # Update with actual PID from jobs output if necessary
run_test "wake 1"
run_test "term 1"

echo "7. Debug Mode" | tee -a $TEST_OUTPUT
run_test "quit"

echo "Tests completed." | tee -a $TEST_OUTPUT

# Display output
echo "Test results saved in $TEST_OUTPUT"
