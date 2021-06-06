.DEFAULT: ioapi

ioapi: ioapi.c Makefile
	gcc -Os -Wall -lwiringPi ioapi.c -o ioapi
