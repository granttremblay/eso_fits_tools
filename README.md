### A liferaft for ESO's old-school FITS tools

I fear change, and these are no longer available on ESO's website. I therefore am preserving them here. 

The twins of FITS header display, dfits and fitsort are very handy to get an overview of what is contained in large amounts of FITS files. dfits dumps FITS headers to its stdout, nothing more. The good side is that it accepts jokers on the command-line, allowing constructs such as: 

`dfits *.fits | grep OBJECT`

To compile: 

`gcc -o dfits dfits.c`
`gcc -o fitsort fitsort.c`

Example usage: 

`dfits *.fits* | fitsort DPR.CATG PRO.CATG EXPTIME IMAGETYP`

`dfits *.fits* | fitsort DPR.CATG EXPTIME IMAGETYP | grep SCIENCE`

