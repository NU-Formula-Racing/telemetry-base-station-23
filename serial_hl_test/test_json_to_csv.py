import json
import csv

# Open JSON file and load data
with open("test.json") as json_file:
  j = json.load(json_file)

# Open CSV file and its writer
csv_file = open("test_data.csv", "w")
csv_out = csv.writer(csv_file)

# Ensures the header is written before data
head_written = False

# Print the header first
if not head_written:
  # Get header names (sensor names)
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

# Write row
csv_out.writerow(data)

# Close file and end program
csv_file.close()