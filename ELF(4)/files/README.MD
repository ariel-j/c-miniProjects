Lab 4: Introduction to ELF (Executable and Linkable Format)

Overview

Lab 4 focuses on analyzing, extracting, and modifying ELF files. This lab combines reverse engineering techniques with binary file manipulation to deepen your understanding of ELF structures. It includes tasks that involve reading ELF headers, parsing data, and modifying binary files.

Lab Goals

Extract useful information from ELF files.

Modify binary files through reverse engineering techniques.

Build and enhance a binary file manipulation tool (hexeditplus).

Parse ELF structures, including section headers and symbol tables.

Prerequisites

Complete Task 0 (0a and 0b) before attending the lab.

Review Tasks 1a, 1b, and 1c in advance.

Familiarity with ELF structures and tools like readelf and hexedit.

Understand basic C programming and debugging techniques.

Methodology

Learn ELF Analysis Tools: Use readelf for human-readable ELF information.

Binary File Editing: Modify files using the custom program hexeditplus.

Debugging Assistance: Utilize debugging messages and tools for verification.

Standard Library Use: Employ the C standard library for ease of implementation.

Tasks

Task 0: ELF Basics and Initial Tool Setup

Task 0a: Analyze an ELF File

Use readelf to extract details about the ELF file abc.

Questions to answer:

What is the entry point address?

How many sections are in the file?

Task 0b: Create hexeditplus

Develop a menu-driven program for binary file manipulation.

Implement options:

Toggle Debug Mode

Set File Name

Set Unit Size

Quit

Task 1: Extend hexeditplus Features

Task 1a: Load Into Memory

Load data from a binary file into memory using specified offsets and lengths.

Ensure proper error handling for invalid inputs.

Task 1b: Toggle Display Mode

Switch between decimal and hexadecimal representations for display.

Task 1c: Display Data

File Display: Display contents of a binary file.

Memory Display: Display memory contents based on user input.

Task 1d: Save Into File

Save modified memory data back into a binary file.

Ensure data consistency and handle file operations securely.

Implementation Notes

Use fgets and sscanf for robust input parsing.

Print debug information, such as offsets and memory states, when debug mode is enabled.

Utilize the C struct to maintain program state and manage data.

Implement error handling for invalid inputs, file errors, and memory overflows.

Tools

readelf: Analyze ELF file headers and sections.

hexedit: View and edit binary file contents.

hexeditplus: Custom program to manipulate binary files.

Testing and Debugging

Verify outputs using readelf and hexedit.

Compare modified files against the original versions.

Use debugging messages to track variable values and offsets.

Additional Notes

All ELF files used are 32-bit binaries. Compile your code accordingly.

Maintain extensibility in hexeditplus to support future features.

Use provided sample code (e.g., units.zip) for handling unit sizes.

Deliverables

Completed answers for Task 0a.

Working implementation of hexeditplus with Task 0b features.

Extended functionality implemented in Task 1 (1a, 1b, 1c, and 1d).

Verification outputs and testing logs.

Conclusion

This lab provides foundational knowledge of ELF structures and practical experience in binary file manipulation. The skills acquired will be extended in future labs to handle more complex ELF processing tasks.

