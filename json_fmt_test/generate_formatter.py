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
import subprocess
from os import getcwd # Get directory bearings
from pathlib import Path # Manipulate to find struct header file

# Import current environment
env = DefaultEnvironment()

### Classes ###
# Abstraction and automation behind parsing struct member names,
# in particular handling formatting automation
class MemberNames:
  def __init__(self, parent: str):
    # Storage list for names
    self.names = []
    
    # Iteration index
    self.iter = 0
    
    # Parent name of member, if any
    # Setting it blank prevents it from appearing
    self.parent = parent
    
  # Add new name
  def append(self, name):
    self.names.append(name)
    
  def len(self):
    return len(self.names)
    
  def __iter__(self):
    return self
    
  def __next__(self):
    if (self.iter >= len(self.names)):
      raise StopIteration
    fmt_line = "  Serial.print(\""
    if self.iter == 0:
      fmt_line += ("}," if self.parent is not "fast" else "") + (("{\\\"" + self.parent + "\\\":{") if self.parent is not "" else "")
    fmt_line += f"{',' if self.iter > 0 else ''}\\\"{self.names[self.iter]}\\\":\"); Serial.print(sv->{self.parent}{'' if self.parent is '' else '.'}{self.names[self.iter]});\n"
    self.iter += 1
    return fmt_line

### Locate files ###
# Filenames
# This must be a C header file containing at least
# the struct alias sensor_vals_t
# and the typedef message_codt_t
# with properties adherent to Telemetry Base Station.
header_name = "sensor_vals.h"

# This must be a C++ program file, complete with the `.cpp` suffix,
# wherein the corresponding header file with the same name
# contains a function (void) (message_codt_t* mc, sensor_vals_t* sv)
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

# Controller to determine
# An enum
active_parse = False

# Identify names of members
for line in header.readlines():
  line_parse = []
  # Identify value struct definition
  if line[:6] == "struct":
    line_parse = line.split(" ")
    # if line_parse
    print(line)
    
  
    
  # If relevant data retrieved, break
  if (struct_def_count > 3 and not active_parse):
    break

# Close header file
header.close()

### Write formatter function ###
print("Writing to ", target_name, "...")

# Since formatter must be changed, the file is reset
if target_path.exists():
  print("Resetting file...")
  subprocess.run(["rm", target_path])
  subprocess.run(["touch", target_path])
if not target_path.exists():
  print("Error: Target not successfully created.")
  exit(1)
  
# Actually open target file
target = open(target_path, "w")

# Write initial lines
# These correspond to including the target's header file and the function definition
target.write(f'#include "{target_name[:-4]}.h"\n\nvoid {target_name[:-4]}(message_code_t* mc, sensor_vals_t* sv)' + ' {\n')

# Test integrity of MemberNames abstraction class
# TODO: Make this relevant to actual sensor data
fast_members.append("jigglypuff")
fast_members.append("sheik")
fast_members.append("greninja")

med_members.append("sunset")
med_members.append("twilight")

slow_members.append("fake_value")

cond_members.append("exodia")
cond_members.append("ragnar")

# Write formatter function
# target.write("  Serial.print(\"{\\\"fast\\\":{\");\n")
for l in fast_members:
  target.write(l)
# if med_members.len() > 0:
#   target.write("  Serial.print(\"},\\\"med\\\":{\");\n")
for l in med_members:
  target.write(l)
# target.write("  Serial.print(\"},\\\"slow\\\":{\");\n")
for l in slow_members:
  target.write(l)
# target.write("  Serial.print(\"},\");\n")
for l in cond_members:
  target.write(l)
target.write("  Serial.println(\"}\");\n")

# Close target file
target.write('}') # Finish function
target.close()
print("Formatter generation complete! Returning to normal build")