# File: generate_formatter.py
# Author: Derek Guo
# Brief: Build script to create a JSON formatter based on the structs in sensor_vals.h
# Version 1
# Date: 2023-02-08
# Copyright (c) 2023

# This is not a pure Python file, but a build script used by PIO.
# Any errors that the Python environment raises can be reasonably ignored unless
# they affect script execution.

# This script must be run "pre" compilation, since the formatting function should
# be completed/updated if it is to be used in the main program.
# If a user is satisfied and wishes not to continue executing the script,
# simply comment out the `extra_scripts` line in `platform.ini`.

### Libraries ###
# Import system libraries
from os import getcwd # Get directory bearings
from pathlib import Path # Manipulate to find struct header file

# Import current environment
env = DefaultEnvironment()

### Classes ###


### Locate files ###
# Filenames
header_name = "sensor_vals.h"
target_name = "sv_fmt_json.cpp"

# Find files in local directory (initialized by PIO)
print("Locating", header_name, "...")
header_path = Path(".") / "include" / header_name
if not header_path.exists():
  print("Error: Header not found.")
  exit(1)

print("Locating", target_name, "...")
target_path = Path(".") / "src" / target_name
if not target_path.exists():
  print("Error: Target not found.")
  exit(1)
  
# Open files
header = open(header_path)
target = open(target_path)

### Parse struct data ###
print("Parsing struct formats...")

# Hold member names, datatypes
fast_members