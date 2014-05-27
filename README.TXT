This inital version 0.1 has the following functionality

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
