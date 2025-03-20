#!/bin/bash

# Function to display test case results
run_test() {
    echo -e "\n[Test $1] $2"
    echo "Command: $3"
    echo "Expected Output: $4"
    echo "Result:"
    eval "$3"
}

# Test 0a: Basic shell prompt and quit command
run_test 0a "Testing shell prompt and quit command" \
    'echo "quit" | ./myshell' \
    "Shell exits when 'quit' is entered."

# Test 0b: Signal handler in looper
gcc -o looper looper.c
echo -e "\n[Test 0b] Testing signal handling with looper"
./looper &
LOOPER_PID=$!
echo "Sending SIGTSTP to $LOOPER_PID"
kill -SIGTSTP $LOOPER_PID
sleep 1
echo "Sending SIGCONT to $LOOPER_PID"
kill -SIGCONT $LOOPER_PID
sleep 1
echo "Sending SIGINT to $LOOPER_PID"
kill -SIGINT $LOOPER_PID
wait $LOOPER_PID 2>/dev/null

# Test 1a: Forking and maintaining shell activity
run_test 1a "Testing shell remains active after command execution" \
    'echo -e "ls\nquit" | ./myshell' \
    "Shell lists files and remains active until 'quit'."

# Test 1b: cd command
mkdir test_dir
run_test 1b "Testing 'cd' command" \
    'echo -e "cd test_dir\npwd\ncd ..\nquit" | ./myshell' \
    "Shell changes directory and prints correct paths."

# Test 1c: Foreground and background processes
run_test 1c "Testing blocking and non-blocking commands" \
    'echo -e "./looper &\nps\nquit" | ./myshell' \
    "Looper process runs in the background."

# Task 2: Signal commands (stop, wake, term)
./looper &
LOOPER_PID1=$!
./looper &
LOOPER_PID2=$!
./looper &
LOOPER_PID3=$!

echo -e "\n[Test 2] Testing process signals"
echo "Stopping process $LOOPER_PID1"
echo "stop $LOOPER_PID1" | ./myshell
sleep 1

echo "Waking process $LOOPER_PID1"
echo "wake $LOOPER_PID1" | ./myshell
sleep 1

echo "Terminating process $LOOPER_PID2"
echo "term $LOOPER_PID2" | ./myshell
sleep 1

echo "Terminating process $LOOPER_PID3"
kill -SIGINT $LOOPER_PID3

wait $LOOPER_PID1 $LOOPER_PID2 $LOOPER_PID3 2>/dev/null

# Cleanup
rm -rf test_dir looper output.txt input.txt
echo -e "\nAll tests completed."

#to compile:  chmod +x test_shell.sh


# Test Cases:

# Task 0a: Ensures myshell displays the prompt and exits on "quit".
# Task 0b: Verifies signal handling in the looper program with SIGTSTP, SIGCONT, and SIGINT.
# Task 1a: Confirms the shell uses fork to maintain interactivity.
# Task 1b: Tests the cd command by creating a temporary directory.
# Task 1c: Checks proper handling of blocking and non-blocking processes.
# Task 2: Sends SIGSTOP, SIGCONT, and SIGINT signals to test the stop, wake, and term commands.