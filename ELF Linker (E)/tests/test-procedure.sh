# Compile the test file
gcc -m32 -c test.c -o test.o

# Compile your ELF program
gcc -m32 -o myELF ELFmenu.c

# Get reference output
readelf -s test.o > readelf_output.txt

# Test cases:
# 1. Basic symbol table reading
./myELF << EOF
1
test.o
3
6
EOF

# 2. Debug mode testing
./myELF << EOF
0
1
test.o
3
6
EOF

# 3. Multiple file testing
gcc -m32 -c -o test2.o test.c
./myELF << EOF
1
test.o
1
test2.o
3
6
EOF