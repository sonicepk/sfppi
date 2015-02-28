default: sfppi-generic sfpp-vendor

sfppi-generic:
	gcc -o sfppi-generic sfppi-generic.c -lwiringPi -lm

sfpp-vendor:
	gcc -o sfpp-vendor sfppi-vendor.c -lwiringPi -lcrypto -lz -lm
