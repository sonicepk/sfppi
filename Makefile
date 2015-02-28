default: sfppi-generic sfppi-vendor

sfppi-generic:
	gcc -o sfppi-generic sfppi-generic.c -lwiringPi -lm

sfppi-vendor:
	gcc -o sfppi-vendor sfppi-vendor.c -lwiringPi -lcrypto -lz -lm

clean:
	$(RM) sfppi-generic sfppi-vendor 
