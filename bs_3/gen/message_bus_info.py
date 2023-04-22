# File: message_bus_info.py
# Author: Derek Guo
# Brief: Script to discern which bus each CAN RX message belongs to
# Version 1
# Date: 2023-04-22
# Copyright (c) 2023

# Import system libraries
from pathlib import Path # Manipulate to find struct header file

# Locate message information file
print("Opening telemetry.cpp ...")
path = Path(".") / "src" / "telemetry.cpp"
if not path.exists(): # For bare-bones projects
  print("Error: file not found.")
  exit(1)
  
# Read through file
print("Locating signal declarations...\n")
file = open(path, "r")
for line in file.readlines():
  # Stop reading once the src file moves into function definitions
  # Convention is that functions come after vars
  if line == "/********** FUNCTIONS **********/\n":
    break
  
  # Parse character blocks between whitespaces
  words = line.split(" ")
  
  # Check if length is sufficient and contains a CAN RX message declaration
  if len(words) > 2 and words[2].startswith("CANRXMessage"):
    # Isolate relevant information: [message_name, bus]
    info = words[3].split("{")
    
    # Print corresponding registration line for telemetry.cpp
    print(f"    {info[1][:-1]}.RegisterRXMessage({info[0]});")
file.close()

print("\nOkie-dokie-lokie!")