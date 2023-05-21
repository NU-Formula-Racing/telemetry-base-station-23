# Data parsing
import serial
import json
import csv

# Interrupt handling
import sys
import time
import signal

# Change to your specific port (esp. on Windows) upon opening this document
# You can do this by looking for where your system stores ports and checking
# which one pops up when you connect the device
port = serial.Serial("/dev/tty.usbmodem115442501", 9600)
print("Connected to Serialport")

# Open CSV file and its writer
csv_file = open("test_data.csv", "w")
csv_out = csv.writer(csv_file)

# Interrupt handler function (to close the file)
def interrupt_handler(signum, frame):
  print("\nClosing CSV...")
  csv_file.close()
  sys.exit(0)
signal.signal(signal.SIGINT, interrupt_handler)

# Ensures the header is written before data
head_written = False

# Use manual interrupt (Ctrl-C) to terminate, because it won't do so otherwise
while True:
  # Checks if a new line is available from serialport; halts until there is one
  # It then decodes the ASCII-encoding into a String and removes the newline characters
  data = port.readline().decode("ascii").strip('\n\r')
  
  # Parse into JSON
  try:
    j = json.loads(data)
  except json.decoder.JSONDecodeError:
    continue
  
  # # Display line (containing the full JSON) and a single JSON query
  # print(data)
  # print(j["fast"]["fl_wheel_speed"])
  
  # Console log to confirm reception
  # print("D")
  
  # Print the header first
  if not head_written:
    # Get header names (sensor names)
    # Assumes JSON structure is constant-size (not changing dynamically)
    header = list(j["fast"])
    header.extend(list(j["slow"]))
    header.extend(list(j)[2:])
    
    # Write header row
    csv_out.writerow(header)
    
    # Prevent header information from being rewritten
    head_written = True
    
  # Get data values (sensor names)
  data = list(j["fast"].values())
  data.extend(list(j["slow"].values()))
  del j["fast"] # Get rid of the sensor data already parsed
  del j["slow"]
  data.extend(list(j.values()))
  
  print(data)

  # Write row
  csv_out.writerow(data)