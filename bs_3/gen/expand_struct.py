# File: expand_struct.py
# Author: Derek Guo
# Brief: Script to create additional fields for a given struct
# Version 1
# Date: 2023-04-20
# Copyright (c) 2023

# Import system libraries
from pathlib import Path # Manipulate to find struct header file

# This is designed to automate member/element/line generation for
# code blocks using field members of a given struct to maintain
# precision and efficiency for our rapidly growing data structs.
# This project is one such example.

# Place your struct in the input file `in.txt`. This should be
# formatted using the basic C struct syntax, NOT using `typedef`.
# In other words, if the head line starts with `struct YOUR_NAME`,
# then you should have it right.

# Then run this program using the command `python3 gen/expand_struct.py``
# inside the *project directory path*, NOT inside `gen/`.
# This is designed for internal use and is not user friendly.

# The output lines will be in the file `out.txt`, and are sectioned
# based on their application. They are formatted such that copying the
# blocks and replacing their obsolete counterparts should be sufficient.
# Only one other piece of output is printed outside of this file, which
# counts the number of members in the struct.

# Find files in directory
print("Reading struct from in.txt...")
in_path = Path(".") / "gen" / "in.txt"
if not in_path.exists(): # Assumes header file is present
  print("Error: Input file not found.")
  exit(1)

# Store member field (type, name)
members = []

fin = open(in_path, "r")
fin.readline()
for line in fin.readlines():
  if len(line) <= 2:
    break
  words = line.split(" ")
  members.append((words[2], words[3].rstrip(";\n")))
fin.close()

# Open output file
print("Processing data...")
out_path = Path(".") / "gen" / "out.txt"
fout = open(out_path, "w")

# Convert members to reference equivalents
fout.write("Refs:\n")  
for m in members:
  fout.write("  " + m[0] + "* " + m[1] + ";\n")
  
# Format sizes
fout.write("\nSizes:\n")
for m in members:
  fout.write("  sizeof(" + m[0] + "),\n")
  
# Get total number of sensors for macro
print("Number of sensors: " + str(len(members)))

# Create initilization statements for `telemetry.cpp`
fout.write("\nInits:\n")
for m in members:
  fout.write("        ." + m[1] + "= &(" + m[1] + "_sig.value_ref()),\n")
  
print("Output write complete.")