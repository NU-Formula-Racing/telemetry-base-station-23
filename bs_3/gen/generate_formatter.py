# File: generate_formatter.py
# Author: Derek Guo
# Brief: Build script to create a JSON formatter based on the structs in sensor_vals.h
# Version 1
# Date: 2023-02-08
# Copyright (c) 2023

# This is not a pure Python file, but a build script used by PIO.
# Any errors that the Python environment raises can be reasonably ignored 
# if otherwise allowed by PIO documentation.

# This script must be run "pre" compilation, since the formatting function should
# be completed/updated if it is to be used in the main program.
# If a user is satisfied and wishes not to continue executing the script,
# simply comment out the `extra_scripts` line in `platform.ini`.

### Libraries ###
# Import system libraries
import subprocess
from os import getcwd # Get directory bearings
from pathlib import Path # Manipulate to find struct header file

### Classes ###
# Abstraction and automation behind parsing struct member names,
# in particular handling formatting automation
class MemberNames:
  def __init__(self, parent: str):
    # Storage list for names, monotonically increases
    self.names = []
    
    # Iteration index for names array
    self.iter = 0
    
    # Parent name of member, if any
    # Setting it blank prevents it from appearing
    self.parent = parent
    
  # Add new name to iterator
  def append(self, name):
    self.names.append(name)
    
  # Get total number of names contained in iterator
  def len(self):
    return len(self.names)
    
  # Used to make this an Iterator
  def __iter__(self):
    return self
   
  # Get next element
  # Increments iter index to advance 
  def __next__(self):
    if (self.iter >= len(self.names)):
      raise StopIteration # Standard for reaching end of Iterator
    
    # All lines start with a tab and the function name
    fmt_line = "  Serial.print(\""
    
    # Additional logic for start of iterator, primarily concerning self.parent
    if self.iter == 0:
      # Assumes "fast" is the first of the parents used, and non-parented variables come up last
      fmt_line += ("}," if self.parent != "fast" else "") + ("{" if self.parent == "fast" else "") + (("\\\"" + self.parent + "\\\":{") if self.parent != "" else "")
    
    # General logic: name, then another Serial.print for the variable
    fmt_line += f"{',' if self.iter > 0 else ''}\\\"{self.names[self.iter]}\\\":\"); Serial.print(sv->{self.parent}{'' if self.parent == '' else '.'}{self.names[self.iter]});\n"
    
    # Advance iterator and return formatted line
    self.iter += 1
    return fmt_line

### Locate files ###
# Filenames
# This must be a C header file containing at least
# the struct alias sensor_vals_t
# and the typedef message_code_t
# with properties adherent to Telemetry Base Station.
header_name = "sensor_vals.h"

# This must be a C++ program file, complete with the `.cpp` suffix,
# wherein the corresponding header file with the same name
# contains a function (void) (message_code_t* mc, sensor_vals_t* sv)
# with the same name as the header that serves as the formatter.
target_name = "sv_fmt_json.cpp"

# Find files in local directory (initialized by PIO)
print("Locating", header_name, "...")
header_path = Path(".") / "include" / header_name
if not header_path.exists(): # Assumes header file is present
  print("Error: Header not found.")
  exit(1)
  
# target_path = header_path / ".." / "src"
# if not target_path.is_dir():
#   print("Error: src directory not found.")
#   exit(1)
target_path = Path(".") / "src" / target_name
  
# target = open(target_path, "w")

### Parse struct formatting ###
print("Parsing struct formats...")

# Open header file
header = open(header_path, "r")

# Hold member names
fast_members = MemberNames("fast")
med_members = MemberNames("med")
slow_members = MemberNames("slow")
cond_members = MemberNames("")

# Control variables
# Count of usable struct definitions found
# For FAST, MED, and SLOW_SENSORS, this increments
# Once it reaches 4, it stops counting
struct_def_count = 0

# Controller to determine whether or not
# the parser is actively collecting member names
# Set to True if a valid struct definition found
# Set to False if searching or end reached
active_parse = False

# Identify names of members
for line in header.readlines():
  # Logic for when search is actively parsing
  if active_parse:
    # End of struct definition
    if line[0] == '}':
      # Increment counter and reset control
      struct_def_count += 1
      active_parse = False
    # FAST_SENSORS
    elif struct_def_count == 0:
      # Identify words in line
      line_parse = line.split(" ")
      if line_parse[2] != "//": # Ignore commented lines
        fast_members.append(line_parse[3][:-2])
    # MED_SENSORS
    elif struct_def_count == 1:
      # For now, these are unused, so just move on
      struct_def_count += 1
      active_parse = False
      # line_parse = line.split(" ")
      # med_members.append(line_parse[3][:-2])
    # SLOW_SENSORS
    elif struct_def_count == 2:
      line_parse = line.split(" ")
      if line_parse[2] != "//":
        slow_members.append(line_parse[3][:-2])
    # SENSOR_VALS conditional members
    elif struct_def_count == 3:
      line_parse = line.split(" ")
      # Ensure parent members aren't counted
      if line_parse[2] != "//" and line_parse[2].find("sensors_t") == -1:
        cond_members.append(line_parse[3][:-2])
  
  # Otherwise, identify value struct definition
  # Assumes structs are declared separate of typedef aliasing
  # i.e. no `typedef_struct`
  if line[:6] == "struct":
    line_parse = line.split(" ")
    # Ignore lines with references
    if line_parse[1].find("REF") == -1:
      active_parse = True
    
  # If relevant data retrieved, stop reading
  if (struct_def_count > 3 and not active_parse):
    break

# Close header file
header.close()

### Write formatter function ###
print("Writing to", target_name, "...")

# Since formatter must be changed, the file is overwritten
if target_path.exists():
  # Previous block overwrites previous file
  print("File reset.")
  subprocess.run(["rm", target_path])
  subprocess.run(["touch", target_path])
  
# Create target file and open it
target = open(target_path, "w")

# Check creation process
if not target_path.exists():
  print("Error: Target not successfully created.")
  exit(1)

# Write initial lines
# These correspond to including the target's header file and the function definition
target.write(f'#include "{target_name[:-4]}.h"\n\nvoid {target_name[:-4]}(message_code_t* mc, sensor_vals_t* sv)' + ' {\n')

# Write formatter function
# Fast member names; also starts up JSON
for l in fast_members:
  target.write(l)
# Med member names
for l in med_members:
  target.write(l)
# Slow member names
for l in slow_members:
  target.write(l)
# Conditional member names
for l in cond_members:
  target.write(l)
# End JSON and start newline
if cond_members.len() == 0:
  target.write("  Serial.println(\"}}\");\n")
else:
  target.write("  Serial.println(\"}\");\n")

# Close target file
target.write('}') # Finish function
target.close()
print("Formatter generation complete! Returning to normal build")