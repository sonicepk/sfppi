/*
*       sfppi-vendor.c
*
* 	Author - eoinpk.ek@gmail.com
*
*	To compile gcc -o sfppi-vendor sfppi-vendor.c -lwiringPi -lcrypto -lz
*	-lm
*
*       sfppi-vendor is free software: you can redistribute it and/or modify
*       it under the terms of the GNU Lesser General Public License as
*       published by the Free Software Foundation, either version 3 of the
*       License, or (at your option) any later version.
*
*       sfppi-vendor is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU Lesser General Public License for more details.
*
*       You should have received a copy of the GNU Lesser General Public
*       License along with sfppi-generic.
*       If not, see <http://www.gnu.org/licenses/>.
***********************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <zlib.h>
#include <openssl/evp.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#define VERSION 0.2

int mychecksum(unsigned char start_byte, unsigned char end_byte);
int dump(char *filename);
int read_eeprom(unsigned char);
int dom(void);
int vendor_fy(void);
int read_sfp(void);
int xio,write_checksum;
unsigned char A50[256]; //only interested in the first 128 bytes
unsigned char A51[256];

int main (int argc, char *argv[])
{
	int opt;
	write_checksum = 0;
	while ((opt = getopt(argc, argv, "rcmd:")) !=-1) {
		switch (opt) {
		case 'r':
			read_sfp();
			break;
		case 'c':
			write_checksum = 1;
			read_sfp();
			vendor_fy();
			break;
		case 'm':
			dom();
			break;
		case 'd':
			dump(optarg);
			break;
		default: /* '?' */
			fprintf(stderr,
			"Usage: %s\n"
			"	-r read the sfp or sfp+\n"
			"	-c calculate and write Vendor bytes\n"
			"	-m Print DOM values if SFP supports DOM\n"
			"	-d filename - dump the eprom to a file\n"
			,argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (argc <=1) {
			fprintf(stderr,
			"Usage: %s\n"
			"	-r read the sfp or sfp+\n"
			"	-c calculate and write Vendor bytes\n"
			"	-m Print DOM values if SFP supports DOM\n"
			"	-d filename - save the eprom to a file\n"
			,argv[0]);
			exit(EXIT_FAILURE);
	}
}
int read_sfp(void)
{
	unsigned char transceiver[8];//8 bytes - address 3 to 10
	unsigned char vendor[16+1]; //16 bytes - address 20 to 35
	unsigned char oui[3]; //3 bytes - address 37 to 39
	unsigned char partnumber[16+1]; //16 bytes - address 40 to 55
	unsigned char revision[4]; //4 bytes - address 56 - 59
	unsigned char serial[16+1]; //16 bytes - address 68 to 83
	unsigned char date[8+1];//8 bytes - address 84 to 91 
	unsigned char vendor_spec[16]; //16 bytes - address 99 to 114
	int cwdm_wave;
	static char *connector[16] = {
                "Unknown",
                "SC",
                "Fibre Channel Style 1 copper connector",
                "Fibre Channel Style 2 copper connector",
                "BNC/TNC",
                "Fibre Channel coaxial headers",
                "FiberJack",
                "LC",
                "MT-RJ",
                "MU",
                "SG",
                "Optical pigtail"};

	printf("SFPpi Version:%0.1f\n\n",VERSION);

	//Copy eeprom SFP details into A50
	if(!read_eeprom(0x50)); else exit(EXIT_FAILURE);

	//print the connector type
	printf("Connector Type = %s",connector[A50[2]]);

	//print the transceiver type
	if(A50[6] & 16) {
		printf("\nTransceiver is 100Base FX");
	}
	else if(A50[6] & 8) {
		printf("\nTransceiver is 1000Base TX");
	}
	else if(A50[6] & 4) {
		printf("\nTransceiver is 1000Base CX");
	}
	else if(A50[6] & 2) {
		printf("\nTransceiver is 1000Base LX");
	}
	else if(A50[6] & 1) {
		printf("\nTransceiver is 1000Base SX");
	}
	else if(A50[3] & 64) {
		printf("\nTransceiver is 10GBase-ER");
	}
	else if(A50[3] & 32) {
		printf("\nTransceiver is 10GBase-LRM");
	}
	else if(A50[3] & 16) {
		printf("\nTransceiver is 10GBase-LR");
	}
	else if(A50[4] & 12) {
		printf("\nTransceiver is 10GBase-ZR");
	}
	else if(A50[3] & 8) {
		printf("\nTransceiver is 10GBase-SR");
	}
	else {
		printf("\nTransceiver is unknown");
	}

        //3 bytes.60 high order, 61 low order, byte 62 is the mantissa
        //print sfp wavelength 
        cwdm_wave = ((int) A50[60]<<8) | ((int) A50[61]);
        printf("\nWavelength = %d.%d",cwdm_wave,A50[62]);

	//print vendor id bytes 20 to 35
	memcpy(&vendor, &A50[20],16);
	vendor[16] = '\0';
	printf("\nVendor = %s",vendor);

	//Print partnumber values address 40 to 55  
	memcpy(&partnumber, &A50[40], 16);
	partnumber[16] = '\0';
	printf("\nPartnumber = %s", partnumber);

	//Print serial values address 68 to 83  
	memcpy(&serial, &A50[68], 16);
	serial[16] = '\0';
	printf("\nSerial = %s", serial);

	//Print date values address 84 to 91  
	memcpy(&date, &A50[84], 8);
	date[8] = '\0';
	printf("\ndate = %s", date);

	if(!write_checksum){
	//Calculate the checksum: Add up the first 31 bytes and store
	//the last 8 bits in the 32nd byte
	mychecksum(0x0, 0x3f);

	//Calculate the extended checksum: Add up 31 bytes and store
	//the last 8 bits in the 32nd byte
	mychecksum(0x40, 0x5f);
	}
	//If Digital Diagnostics is enabled and is Internally calibrated print
	//the DOM values.
	if (A50[92] & 0x60){
		dom();
	}
return 0;
}

int dump(char *filename)
{
	int j;
        unsigned char index_start = 0;
	unsigned char index_end = 0;
        int i = 0;
        int counter = 0x0;
        FILE *fp;

	//Copy eeprom SFP details into A50
	if(!read_eeprom(0x50)); else exit(EXIT_FAILURE);

	fp=fopen(filename,"w");
        if (fp == NULL) {
                 fprintf (stderr, "File not created okay, errno = %d\n",
				 strerror (errno));
                return 1;
        }
        printf ("Dump eeprom contents to %s\n",filename) ;
        fprintf(fp,"     0   1   2   3   4   5   6   7   8   9   a   b  "
		       " c   d   e   f   0123456789abcdef");
        while (counter < 0x100){ //address 0 to 255
		if ((counter % 0x10) == 0){
                       index_end = counter;
                        fprintf(fp,"  ");
                        for(j = index_start; j <index_end; j++) {
                                if(A50[index_start] == 0x0 ||
                                        A50[index_start] == 0xff)
                                        fprintf(fp,".");
                                else
                                if (A50[index_start] < 32 ||
                                        A50[index_start] >=127){
                                        fprintf(fp,"?");
                                }
                                else
                                fprintf(fp,"%c",A50[index_start]);
                                index_start++;
                        }
                        index_start = index_end;
                        fprintf(fp,"\n%02x:",i);
			i = i + 0x10;
                }
                fprintf(fp,"  %02x",A50[counter]);
                counter = counter + 1;
        }
        fclose(fp);
        return 0;
}

int read_eeprom(unsigned char address)
{
	int xio,i,fd1;
        xio = wiringPiI2CSetup (address);
	if (xio < 0){
                fprintf (stderr, "xio: Unable to initialise I2C: %s\n",
                                strerror (errno));
                return 1;
        }
        /*Read in the first 128 bytes 0 to 127*/
        for(i=0; i <128; i++){
                fd1 = wiringPiI2CReadReg8 (xio,i);
                if  (address == 0x50){
			A50[i] = fd1;
		}
		else{
		       A51[i] = fd1;
		}
		if (fd1 <0){
                fprintf (stderr, "xio: Unable to read i2c address 0x%x: %s\n",
				address, strerror (errno));
			      return 1;
			}
	}
	return 0;
}

int dom(void)
{
	//102 TX MSB, 103 TX LSB, 104 RX MSB, 105 RX LSB.
	float temperature, vcc, tx_bias, optical_rx, optical_tx;

	if(!read_eeprom(0x51)); else exit(EXIT_FAILURE);

	temperature = (A51[96] + (float) A51[97]/256);
	vcc = (float)(A51[98]<<8 | A51[99]) * 0.0001;
	tx_bias = (float)(A51[100]<<8 | A51[101]) * 0.002;
	optical_tx = 10 * log10((float)(A51[102]<<8 | A51[103]) * 0.0001);
	optical_rx = 10 * log10((float)(A51[104]<<8 | A51[105]) * 0.0001);
	//Print the results
	printf ("Internal SFP Temperature = %4.2fC\n", temperature);
	printf ("Internal supply voltage = %4.2fV\n", vcc);
	printf ("TX bias current = %4.2fmA\n", tx_bias);
	printf ("Optical power Tx = %4.2f dBm\n", optical_tx);
	printf ("Optical power Rx = %4.2f dBm\n", optical_rx);

	return 0;
}

int vendor_fy(void)
{
	//You need to add a valid 16 byte vendor key in Hex
	unsigned char vendor_key1[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char man_id[49];
	unsigned long crc_32;
	unsigned char vendor_crc[4];
	unsigned char vendor_id[16];
	unsigned char serial_id[16];
	int i;

	//Copy eeprom SFP details into A50
	if(!read_eeprom(0x50)); else exit(EXIT_FAILURE);

	memcpy(&vendor_id, &A50[20],16);
	memcpy(&serial_id, &A50[68],16);

	//vendor_tmp holds = man_id 1 byte + 16 byte vendor + 16 byte serial + 
	//  16 byte vendor_key1
	man_id[0] = 0x00; //You need to provide a manufacturer id number. 
	for(i = 1; i <17; i++)
		man_id[i] = vendor_id[i-1];
	for(i = 17; i <33; i++)
		man_id[i] = serial_id[i-17];
	for(i = 33; i <49; i++)
		man_id[i] = vendor_key1[i-33];
	//need to calculate the md5 of the concatenated string
	//using openssl envelope functions from openssl libcrypto
	EVP_MD_CTX *mdctx;
        const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
        int md_len;

	OpenSSL_add_all_digests();
	mdctx = EVP_MD_CTX_create();
		md = EVP_get_digestbyname("md5");
		EVP_DigestInit_ex(mdctx, md, NULL);
		EVP_DigestUpdate(mdctx, man_id, 49);
		EVP_DigestFinal_ex(mdctx, md_value, &md_len);
        //clean up
		EVP_MD_CTX_destroy(mdctx);

	printf("\nDigest is: ");
	for(i = 0; i < md_len; i++) printf("%02x", md_value[i]);

	//Create valid id
		    unsigned char vendor_trailer[9+1] = {0x00,0x00,0x00,0x00,0x00
							,0x00,0x00,0x00,0x00};
		    unsigned char vendor_valid_id[28+1];
		    vendor_valid_id[0] = 0x00;
		    vendor_valid_id[1] = 0x00;
		    vendor_valid_id[2] = 0x02;
		    for(i = 0; i < 16; i++)
			    vendor_valid_id[i+3] = md_value[i];
		    for(i = 0; i < 10; i++)
			    vendor_valid_id[i+19] = vendor_trailer[i];
		    printf("\nVendor Valid Id = ");
		    for(i=0; i < 29; i++) printf("%x",vendor_valid_id[i]);

	//write the vendor_valid_id from address 96(0x60) to address 124(0x7b) 
	printf("\nWrite valid_vendor_id Yes/No?");
		int ch = 0;
		ch = getchar();
		getchar();
		if ((ch == 'Y') || (ch == 'y'))
		{
			printf("Writing Digest wait....\n");
			xio = wiringPiI2CSetup (0x50);
				if (xio < 0){
				fprintf (stderr, "xio: Unable to initialise I2C: %s\n",
                                strerror (errno));
				return 1;
				}
			for(i = 0; i < 28; i++) {
			wiringPiI2CWriteReg8(xio, 0x60+i, vendor_valid_id[i]);
			usleep(50000);//sleep for 0.5ms per byte 
			}
		} else printf("nothing written");

	//now need to get the crc32 of the header+md5+trailer
	crc_32 = crc32(0, vendor_valid_id, 28);

	//printf("\nvalue of returned crc = %x",crc_32);
		vendor_crc[0] = (int) crc_32 & 0xff; //A50[124]
		vendor_crc[1] = (int) crc_32 >> 8 & 0xff; //A50[125]
		vendor_crc[2] = (int) crc_32 >> 16 & 0xff;//A50[126]
		vendor_crc[3] = (int) crc_32 >> 24 & 0xff;//A50[127]
		printf("\nCRC32 of the Vendor Padded MD5 =");
		for(i = 0; i < 4; i++) printf(" %x", vendor_crc[i]);
	//need to write the crc values to the eeprom
	//
	printf("\nWrite CRC32 of the padded Digest in reverse (4 seconds) Yes/No?");
		ch =0;
		ch = getchar();
		getchar();
		if ((ch == 'Y') || (ch == 'y'))
		{
			printf("Writing CRC32 wait....\n");
			xio = wiringPiI2CSetup (0x50);
				if (xio < 0){
				fprintf (stderr, "xio: Unable to initialise I2C: %s\n",
                                strerror (errno));
				return 1;
				}
			for(i = 0; i < 4; i++)
			{
			wiringPiI2CWriteReg8(xio, 0x7c+i,vendor_crc[i]);
			usleep(50000);//wait 0.5ms per byte
			}
		} else printf("nothing written\n");
	if (write_checksum)
	{
	//Calculate the checksum: Add up the first 31 bytes and store
	//the last 8 bits in the 32nd byte
	mychecksum(0x0, 0x3f);

	//Calculate the extended checksum: Add up 31 bytes and store
	//the last 8 bits in the 32nd byte
	mychecksum(0x40, 0x5f);
	}
	return 0;
}

int mychecksum(unsigned char start_byte, unsigned char end_byte)
{
	int sum = 0;
	int counter;
	int cc_base;
	for (counter = start_byte; counter <end_byte; counter++)
	sum = (A50[counter] + sum);
	sum = sum & 0x0ff;
	cc_base = A50[end_byte]; //sum stored in address 63 or 95.
	if (start_byte == 0x0) printf("\ncc_base = %x, sum = %x",cc_base,sum);
	else printf("\nextended cc_base = %x and sum = %x\n",cc_base,sum);
		if (cc_base != sum && write_checksum){
			printf("\nCheck Sum failed, Do you want to write the"
			       "	checksum value %x to address byte \"%x\" ?"
			       "	(Y/N)", sum, end_byte);
			int ch = 0;
			ch = getchar();
			getchar();
			if ((ch == 'Y') || (ch == 'y')) {
				xio = wiringPiI2CSetup (0x50);
				if (xio < 0){
				fprintf (stderr, "xio: Unable to initialise I2C: %s\n",
                                strerror (errno));
				return 1;
				}
				printf("end_byte = %x and sum = %x - yes\n",end_byte, sum);
				wiringPiI2CWriteReg8(xio, end_byte, sum);
			} else printf("nothing written\n");
		}
		return 0;
}
