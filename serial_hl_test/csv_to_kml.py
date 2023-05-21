import csv
import xml.dom.minidom
import sys

def extractAddress(row):
  # This extracts an address from a row and returns it as a string. This requires knowing
  # ahead of time what the columns are that hold the address information.
  return '%s,%s' % (row['latitude'], row['longitude'])

def createPlacemark(kmlDoc, row, order):
  # This creates a  element for a row of data.
  # A row is a dict.
  placemarkElement = kmlDoc.createElement('Placemark')
  extElement = kmlDoc.createElement('ExtendedData')
  placemarkElement.appendChild(extElement)
  
  # Loop through the columns and create a  element for every field that has a value.
  for key in order:
    if row[key]:
      dataElement = kmlDoc.createElement('Data')
      dataElement.setAttribute('name', key)
      valueElement = kmlDoc.createElement('value')
      dataElement.appendChild(valueElement)
      valueText = kmlDoc.createTextNode(row[key])
      valueElement.appendChild(valueText)
      extElement.appendChild(dataElement)
  
  pointElement = kmlDoc.createElement('Point')
  placemarkElement.appendChild(pointElement)
  coordinates = extractAddress(row)
  coorElement = kmlDoc.createElement('coordinates')
  coorElement.appendChild(kmlDoc.createTextNode(coordinates))
  pointElement.appendChild(coorElement)
  return placemarkElement

def createKML(csvReader, fileName, order):
  # This constructs the KML document from the CSV file.
  kmlDoc = xml.dom.minidom.Document()
  
  kmlElement = kmlDoc.createElementNS('http://earth.google.com/kml/2976459236452967345234265798', 'kml')
  kmlElement.setAttribute('xmlns','http://earth.google.com/kml/2976459236452967345234265798')
  kmlElement = kmlDoc.appendChild(kmlElement)
  documentElement = kmlDoc.createElement('Document')
  documentElement = kmlElement.appendChild(documentElement)

  # Skip the header line.
  csvReader.__next__()
  
  for row in csvReader:
    row = {order[0]: row[order[0]], order[1]: row[order[1]]}
    placemarkElement = createPlacemark(kmlDoc, row, order)
    documentElement.appendChild(placemarkElement)
  kmlFile = open(fileName, 'w')
  kmlFile.write(kmlDoc.toprettyxml('  ', newl = '\n'))

def main():
  # This reader opens up 'google-addresses.csv', which should be replaced with your own.
  # It creates a KML file called 'google.kml'.
  
  # If an argument was passed to the script, it splits the argument on a comma
  # and uses the resulting list to specify an order for when columns get added.
  # Otherwise, it defaults to the order used in the sample.
  
  if len(sys.argv) >1:
    order = sys.argv[1].split(',')
  else:
    order = ['latitude','longitude']
  csvreader = csv.DictReader(open('test_data.csv'))
  kml = createKML(csvreader, 'test_data.kml', order)

if __name__ == '__main__':
  main()