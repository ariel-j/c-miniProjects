#!/bin/bash

# Start the shell
echo "Starting shell test..." 

# Test 1: Run simple command (ls)
echo "Test 1: Running 'ls' command"
echo "Expected output: List of files in current directory"
./myshell << EOF
ls
EOF

# Test 2: Check redirection of output to a file
echo "Test 2: Output redirection (echo > output.txt)"
echo "Expected output: 'Hello, World!' in 'output.txt'"
./myshell << EOF
echo "Hello, World!" > output.txt
EOF
echo "Contents of output.txt:"
cat output.txt

# Test 3: Input redirection from a file
echo "Test 3: Input redirection (< input.txt)"
echo "Expected output: 'This is a test file' from 'input.txt'"
echo "This is a test file" > input.txt
./myshell << EOF
cat < input.txt
EOF

# Test 4: Run command with piping (ls | grep 'test')
echo "Test 4: Piping output of ls | grep 'test'"
echo "Expected output: Files that contain 'test' in their name"
./myshell << EOF
ls | grep test
EOF

# Test 5: Execute the shell itself (./myshell)
echo "Test 5: Running the shell itself"
echo "Expected output: Should execute myshell again."
# In this test case, running the shell inside itself can cause the script to hang
# So instead, let's avoid nesting and manually check for shell behavior:
echo "Please manually run ./myshell inside the shell to check this feature."

# Test 6: Run quit command to exit the shell
echo "Test 6: Running quit command"
echo "Expected output: Exit the shell"
./myshell << EOF
quit
EOF

# Test 7: Check for command not found error
echo "Test 7: Running a non-existent command"
echo "Expected output: 'command not found'"
./myshell << EOF
nonexistent_command
EOF

# End of tests
echo "All tests completed."

#to compile:  chmod +x test_shell.sh
