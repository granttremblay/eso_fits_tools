### A life raft for ESO's old-school FITS tools

I fear change, and these are no longer available on ESO's website. I therefore am preserving them here. 

`dfits` and `fitsort` — both beautifully written by Nicolas Devillard at ESO between 1996 and 2000 (!) — dump and filter FITS header contents on the command line. They accept all the usual `*nix` shell stuff, allowing constructs such as: 

`dfits *.fits | grep OBJECT`

`dfits *.fits* | fitsort DPR.CATG PRO.CATG EXPTIME IMAGETYP`

`dfits *.fits* | fitsort DPR.CATG EXPTIME IMAGETYP | grep SCIENCE`

#### To compile: 

`gcc -o dfits dfits.c`

`gcc -o fitsort fitsort.c`

or 

`make; make install` (thanks to Gabe Brammer for this makefile)




