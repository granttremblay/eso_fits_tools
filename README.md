# eso_fits_tools



The twins of FITS header display, dfits and fitsort are very handy to get an overview of what is contained in large amounts of FITS files. dfits dumps FITS headers to its stdout, nothing more. The good side is that it accepts jokers on the command-line, allowing constructs such as:

'''dfits *.fits | grep OBJECT'''