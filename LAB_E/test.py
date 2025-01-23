#!/usr/bin/python3
import subprocess
import sys
import os
from datetime import datetime

class LabTester:
    def __init__(self):
        self.log_file = f"lab_test_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        
    def log(self, message):
        with open(self.log_file, 'a') as f:
            f.write(f"{message}\n")
        print(message)

    def run_myelf(self, commands):
        try:
            process = subprocess.Popen(
                ['./myELF'],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            stdout, stderr = process.communicate(input=commands)
            return stdout, stderr
        except Exception as e:
            return "", str(e)

    def run_readelf(self, args, filename):
        try:
            process = subprocess.Popen(
                ['readelf'] + args + [filename],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            stdout, stderr = process.communicate()
            return stdout, stderr
        except Exception as e:
            return "", str(e)

    def test_single_file(self, filename):
        self.log(f"\n=== Testing file: {filename} ===\n")
        
        # Test sections
        self.log("Testing sections output:")
        commands = f"1\n{filename}\n2\n6\n"
        myelf_out, myelf_err = self.run_myelf(commands)
        readelf_out, _ = self.run_readelf(['-S'], filename)
        
        self.log("MyELF output:")
        self.log(myelf_out)
        if myelf_err:
            self.log(f"MyELF errors:\n{myelf_err}")
        self.log("\nReadelf output:")
        self.log(readelf_out)
        
        # Test symbols
        self.log("\nTesting symbols output:")
        commands = f"1\n{filename}\n3\n6\n"
        myelf_out, myelf_err = self.run_myelf(commands)
        readelf_out, _ = self.run_readelf(['-s'], filename)
        
        self.log("MyELF output:")
        self.log(myelf_out)
        if myelf_err:
            self.log(f"MyELF errors:\n{myelf_err}")
        self.log("\nReadelf output:")
        self.log(readelf_out)

    def test_merge(self, file1, file2):
        self.log(f"\n=== Testing merge of {file1} and {file2} ===\n")
        
        # Test merge check
        self.log("Testing merge check:")
        commands = f"1\n{file1}\n1\n{file2}\n4\n6\n"
        myelf_out, myelf_err = self.run_myelf(commands)
        self.log("Merge check output:")
        self.log(myelf_out)
        if myelf_err:
            self.log(f"Errors:\n{myelf_err}")
        
        # Test actual merge
        self.log("\nTesting merge operation:")
        commands = f"1\n{file1}\n1\n{file2}\n5\n6\n"
        myelf_out, myelf_err = self.run_myelf(commands)
        self.log("Merge operation output:")
        self.log(myelf_out)
        if myelf_err:
            self.log(f"Errors:\n{myelf_err}")
        
        # If merge created output file, compare with reference
        if os.path.exists('out.ro'):
            self.log("\nComparing merged output with reference:")
            readelf_out, _ = self.run_readelf(['-a'], 'out.ro')
            self.log("\nMerged file (out.ro) contents:")
            self.log(readelf_out)
            
            if os.path.exists('F12a.ro'):
                reference_out, _ = self.run_readelf(['-a'], 'F12a.ro')
                self.log("\nReference file (F12a.ro) contents:")
                self.log(reference_out)

def main():
    tester = LabTester()
    
    # Test individual files
    test_files = ['F1a.o', 'F1b.o', 'F1c.o', 'F2a.o', 'F2b.o']
    for file in test_files:
        if os.path.exists(file):
            tester.test_single_file(file)
        else:
            tester.log(f"Warning: {file} not found")
    
    # Test merges
    merge_pairs = [
        ('F1a.o', 'F2a.o'),  # Should succeed
        ('F1b.o', 'F2b.o')   # Should show errors
    ]
    
    for file1, file2 in merge_pairs:
        if os.path.exists(file1) and os.path.exists(file2):
            tester.test_merge(file1, file2)
        else:
            tester.log(f"Warning: Cannot test merge of {file1} and {file2} - files not found")

if __name__ == "__main__":
    main()