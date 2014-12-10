import xml.etree.ElementTree as ET
tree = ET.parse("vendorMacs.xml")
root = tree.getroot()
mac = raw_input("Enter your Mac Address: ")
vendor = mac.upper()[:8]
for elem in root :
	if elem.get("mac_prefix") == vendor:
		print elem.get("vendor_name")
