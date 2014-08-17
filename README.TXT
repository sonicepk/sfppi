This inital version 0.1 of sfp-generic.c has the following functionality.

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
Calculates the checksums and if they are different to what is on the eeprom you will be prompted to write the correct checksum or not. 

---------------------
This inital version 0.1 of sfp-vendor.c has the following functionality.

Usage: ./sfpp-vendor
	-r read the sfp or sfp+
	-c calculate and write Vendor bytes
	-d filename - save the eprom to a file

Note: you need to provide your own valid manufacturer key (vendor_key1) and also a valid manufacturer id man_id[0].
