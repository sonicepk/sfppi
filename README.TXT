This inital version 0.2 of sfp-generic.c has the following functionality.

sfppi -r: 
Prints out the values for some of the eeprom fields. For example:

Connector Type = LC
Transceiver is 1000Base SX
Wavelength = 850.0
Vendor = Raspberry Pi    
Partnumber = QFBR-5766LP     
Serial = 123456789JK     
date = 050310  
cc_base = c6, sum = c6
extended cc_base = 95 and sum= 95 

sfp -d filename.txt:
Dumps the contents of the eprom to a file.

sfppi -c:
Calculates the checksums and if they are different to what is on the eeprom you will be prompted to write the correct checksum or not. Use at your own risk.

sfppi -m:
Prints out some of the Digital Diagnostics Monitoring parameters. SFP must support DOM.
Internal SFP Temperature = 37.66C
Internal supply voltage = 3.26V
TX bias current = 0.00mA
Optical power Tx = -40.00 dBm
Optical power Rx = -10.17 dBm

---------------------
Version 0.21 of sfp-vendor.c has the following functionality.

Usage: ./sfpp-vendor
	-r read the sfp or sfp+
	-c calculate and write Vendor bytes
	-m Print DOM values if SFP supports DOM
	-d filename - save the eprom to a file

Note: you need to provide your own valid manufacturer key (vendor_key1) and also a valid manufacturer id man_id[0].
