BEAGLEBONE =aa

default: sfppi-generic sfppi-vendor

sfppi-generic:
ifndef BEAGLEBONE
	gcc -o sfppi-generic sfppi-generic.c -lwiringPi -lm
else
	gcc -DBEAGLEBONE -o sfppi-generic sfppi-generic.c -lm
endif

sfppi-vendor:
ifndef BEAGLEBONE
	gcc -o sfppi-vendor sfppi-vendor.c -lwiringPi -lcrypto -lz -lm
else
	gcc -DBEAGLEBONE -o sfppi-vendor sfppi-vendor.c -lcrypto -lz -lm
endif

clean:
	$(RM) sfppi-generic sfppi-vendor 
