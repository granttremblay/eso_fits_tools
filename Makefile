CC=gcc

all: dfits fitsort

dfits: dfits.c
	$(CC) -o dfits dfits.c

fitsort: fitsort.c
	$(CC) -o fitsort fitsort.c

install: 
	cp dfits fitsort /usr/local/bin/
