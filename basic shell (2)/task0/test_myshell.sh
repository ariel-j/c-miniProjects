#!/bin/bash

# Define the shell executable
SHELL_EXEC="./myshell"

# Function to test a single command
run_test() {
    local input="$1"
    local expected="$2"

    echo "Test: $input"
    echo -e "$input\nquit" | $SHELL_EXEC > output.log 2>&1

    if grep -q "$expected" output.log; then
        echo "✅ Passed"
    else
        echo "❌ Failed"
        echo "Expected: $expected"
        echo "Got:"
        cat output.log
    fi
    echo "----------------------------------"
}

# Test 1: Basic Command
run_test "/bin/echo Hello, world!" "Hello, world!"

# Test 2: Invalid Command
run_test "invalidcommand" "No such file or directory"

# Test 3: Quit Command
run_test "quit" ""

# Test 4: Current Directory
run_test "pwd" "$(pwd)"

# Test 5: Chaining Commands (requires handling pipes in later tasks)
run_test "/bin/ls | /bin/grep myshell" "myshell"

# Test 6: Input Redirection (requires handling redirection in later tasks)
echo "This is a test file" > test_input.txt
run_test "/bin/cat < test_input.txt" "This is a test file"

# Test 7: Output Redirection (requires handling redirection in later tasks)
run_test "/bin/echo Redirected output > test_output.txt" ""
if [ -f test_output.txt ] && grep -q "Redirected output" test_output.txt; then
    echo "✅ Passed: Output Redirection"
else
    echo "❌ Failed: Output Redirection"
fi

# Clean up temporary files
rm -f test_input.txt test_output.txt output.log


#to compilr: chmod +x test_myshell.sh
