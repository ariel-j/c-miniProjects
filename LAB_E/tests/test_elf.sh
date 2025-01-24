#!/bin/bash

echo "=== Starting ELF Symbol Table Tests ==="

# Create test file
echo "Creating test.c..."
cat > test.c << 'EOF'
int global_var = 42;

void test_function() {
    int local_var = 10;
}

int main() {
    test_function();
    return 0;
}
EOF

# Compile files
echo "Compiling files..."
gcc -m32 -c test.c -o test.o
gcc -m32 -o myELF ELFmenu.c

if [ ! -f myELF ]; then
    echo "❌ Failed to compile myELF program!"
    exit 1
fi

# Test 1: Basic functionality
echo -e "\n=== Test 1: Basic Symbol Table Test ==="
echo -e "1\ntest.o\n3\n6\n" | ./myELF > test_output.txt

# Get readelf output
readelf -s test.o > readelf_output.txt

# Compare number of symbols
symbols_your=$(grep -c "^\[" test_output.txt)
symbols_readelf=$(grep -c "^     [0-9]:" readelf_output.txt)

echo "=== Test Results ==="
echo "Your program symbol count: $symbols_your"
echo "readelf symbol count: $symbols_readelf"

if [ "$symbols_your" -eq "$symbols_readelf" ]; then
    echo "✅ Symbol count matches!"
else
    echo "❌ Symbol count mismatch!"
fi

# Test 2: Debug mode
echo -e "\n=== Test 2: Debug Mode Test ==="
echo -e "0\n1\ntest.o\n3\n6\n" | ./myELF > debug_output.txt

if grep -q "Debug Info" debug_output.txt; then
    echo "✅ Debug info is present"
else
    echo "❌ Debug info missing"
fi

if grep -q "Symbol table size:" debug_output.txt; then
    echo "✅ Symbol table size info present"
else
    echo "❌ Symbol table size info missing"
fi

# Print outputs for manual verification
echo -e "\n=== Detailed Output Comparison ==="
echo "Your program output (test_output.txt):"
cat test_output.txt
echo -e "\nreadelf output (readelf_output.txt):"
cat readelf_output.txt

echo -e "\n=== Test files generated ==="
echo "- test.c: Test source file"
echo "- test.o: Compiled test file"
echo "- test_output.txt: Your program output"
echo "- debug_output.txt: Your program debug output"
echo "- readelf_output.txt: readelf reference output"