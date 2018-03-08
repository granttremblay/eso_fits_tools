
/*----------------------------------------------------------------------------
   
   File name    :   fitsort.c
   Author       :   Nicolas Devillard
   Created on   :   May 1st, 1996
   Description  :   Sorts out FITS keywords.
                    The input is a succession of FITS headers delivered
                    through stdin by 'dfits'. On the command line, specify
                    which keywords you wish to display, the output contains
                    for each file, the file name and the keyword values
                    in column format.
                    Example:
                    dfits *.fits | fitsort BITPIX NAXIS NAXIS1 NAXIS2 NAXIS3
                    The output would be like:

File            BITPIX      NAXIS    NAXIS1    NAXIS2    NAXIS3

image1.fits         32          2       256       256
image2.fits        -32          3       128       128        40
...

					Using 'fitsort -d ...' would prevent printing the
					first line (filename and keyword names).

					The output format is simple: values are separated by
					tabs, records by linefeeds. When no value is present
					(no keyword in this header), only a tab is printed
					out.

Example:
	file1.fits contains NAXIS1=100 NAXIS2=200
	file2.fits contains NAXIS1=20

	dfits file1.fits file2.fits | fitsort NAXIS2 NAXIS1
	would litterally print out (\t stands for tab, \n for linefeed):

file1.fits\t200\t100\n
file2.fits\t\t20\n

 ---------------------------------------------------------------------------*/

/*
	$Id: fitsort.c,v 1.1 2001/12/14 09:15:19 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/12/14 09:15:19 $
	$Revision: 1.1 $
 */

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*----------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

#define MAX_STRING  128        /* Maximum string length                    */
#define MAX_KEY     512         /* Maximum number of keywords to search for */

#define FMT_STRING "%%-%ds\t"

/*----------------------------------------------------------------------------
                                New types   
 ---------------------------------------------------------------------------*/

/* This holds a keyword value and a flag to indicate its presence   */

typedef struct _KEYWORD_ {
    char    value[MAX_STRING] ;
    int     present ;
} keyword ;

/*
 * Each detected file in input has such an associated structure, which
 * contains the file name and a list of associated keywords.
 */

typedef struct _RECORD_ {
    char            filename[MAX_STRING] ;
    keyword         listkw[MAX_KEY] ;
} record ;

/*
 * NB: Everything is deliberately allocated statically. As we are not
 * dealing with huge amounts of data, works fine and makes it easier to
 * read/write/modify.
 */

/*----------------------------------------------------------------------------
                            Function prototypes
                        See function descriptions below
 ---------------------------------------------------------------------------*/

static int
isfilename(char *string) ;

static void 
getfilename(char *line, char *word) ;

static char *
expand_hierarch_keyword(
	char	*	dotkey,
	char	*	hierarchy
);

static int 
isdetectedkeyword(char *line, char *keywords[], int nkeys) ;

static void 
getkeywordvalue(char *line, char *word) ;

/*----------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char    curline[MAX_STRING] ;
    char    word[MAX_STRING] ;
    int     i, j ;
    int     nfiles ;
    record  *allrecords ;
    int     kwnum ;
	int		len ;
	int		max_width[MAX_KEY] ;
	int		max_filnam ;
	char	fmt[8] ;
	int		flag ;
	int		printnames ;
	int		print_hdr ;

    if (argc<2) {
        printf("\n\nuse : %s [-d] KEY1 KEY2 ... KEYn\n", argv[0]) ;
        printf("Input data is received from stdin\n") ;
        printf("See man page for more details and examples\n\n") ;
        return 0 ;
    }

	printnames = 0 ;
	print_hdr  = 1 ;
    nfiles = 0 ;
    allrecords = (record*)calloc(1, sizeof(record));
	if (!strcmp(argv[1], "-d")) {
		print_hdr = 0;
		argv++ ;
		argc-- ;
	}
    argv++ ;

	/* Uppercase all inputs */
	for (i=0 ; i<(argc-1) ; i++) {
		j=0 ;
		while (argv[i][j]!=0) {
			argv[i][j] = toupper(argv[i][j]);
			j++ ;
		}
	}

    while (fgets(curline, MAX_STRING, stdin) != (char*)NULL) {
        if ((flag=isfilename(curline))!=0) {
			/* New file entry is detected */
			if (flag==1) {
				/* New file name is detected, get the new file name */
				printnames = 1 ;
				getfilename(curline, allrecords[nfiles].filename) ;
				/* Absorb next line (contains SIMPLE=) */
				fgets(curline, MAX_STRING, stdin);
			} else {
				/* New SIMPLE=T entry, no associated file name */
				allrecords[nfiles].filename[0] = (char)0;
			}
            nfiles++ ;
			/*
			 * Initialize a new record structure to store input data for
			 * this file.
			 */

            allrecords = (record*)realloc(  allrecords,
                                            (nfiles+1)*sizeof(record)) ;
            for (i=0 ; i<MAX_KEY ; i++)
                allrecords[nfiles].listkw[i].present = 0 ;
		} else {
			/* Is not a file name, is it a searched keyword?    */
            if ((kwnum = isdetectedkeyword(	curline,
											argv,
											argc-1)) != -1) {
				/* Is there anything allocated yet to store this? */
				if (nfiles>0) {
					/* It has been detected as a searched keyword.  */
					/* Get its value, store it, present flag up     */
					getkeywordvalue(curline, word) ;
					strcpy(allrecords[nfiles-1].listkw[kwnum].value, word) ;
					allrecords[nfiles-1].listkw[kwnum].present ++ ;
				}
            }
        }
    }
	for (i=0 ; i<argc-1 ; i++) {
		max_width[i] = (int)strlen(argv[i]) ;
	}

	/* Record the maximum width for each column */
	max_filnam = 0 ;
	for (i=0 ; i<nfiles ; i++) {
		len = (int)strlen(allrecords[i].filename) ;
		if (len>max_filnam) max_filnam=len ;
		for (kwnum=0 ; kwnum<argc-1 ; kwnum++) {
			if (allrecords[i].listkw[kwnum].present) {
				len = (int)strlen(allrecords[i].listkw[kwnum].value) ;
			} else {
				len = 0 ;
			}
			if (len>max_width[kwnum])
				max_width[kwnum] = len ;
		}
	}

	/* Print out header line */
	if (print_hdr) {
		sprintf(fmt, FMT_STRING, max_filnam) ;
		if (printnames) printf(fmt, "FILE");
		for (i=0 ; i<argc-1 ; i++) {
			sprintf(fmt, FMT_STRING, max_width[i]) ;
			printf(fmt, argv[i]) ;
		}
		printf("\n") ;
	}


    /* Now print out stored data    */
	if (nfiles<1) {
		printf("*** error: no input data corresponding to dfits output\n");
		return -1 ;
	}
    for (i=0 ; i<nfiles ; i++) {
		if (printnames) {
			sprintf(fmt, FMT_STRING, max_filnam) ;
			printf(fmt, allrecords[i].filename) ;
		}
        for (kwnum=0 ; kwnum<argc-1 ; kwnum++) {
			sprintf(fmt, FMT_STRING, max_width[kwnum]);
            if (allrecords[i].listkw[kwnum].present)
				printf(fmt, allrecords[i].listkw[kwnum].value) ;
            else
				printf(fmt, " ");
        }
        printf("\n") ;
    }
    free(allrecords) ;
    return 0 ;
}


/*----------------------------------------------------------------------------
   Function :   isfilename()
   In       :   dfits output line
   Out      :   integer
   				1 if the line contains a valid file name as produced
				by dfits.
				2 if the line starts with 'SIMPLE  ='
				0 else
   Job      :   find out if an input line contains a file name or a
   				FITS magic number
   Notice   :
   Filename recognition is based on 'dfits' output.
 ---------------------------------------------------------------------------*/

static int 
isfilename(char *string)
{
	if (!strncmp(string, "====>", 5)) return 1 ;
	if (!strncmp(string, "SIMPLE  =", 9)) return 2 ;
	return 0 ;
}


/*----------------------------------------------------------------------------
   Function :   getfilename()
   In       :   dfits output line
   Out      :   second argument: file name
   Job      :   returns a file name from a dfits output line
   Notice   :   This is dfits dependent.
 ---------------------------------------------------------------------------*/

static void
getfilename(char *line, char *word)
{
    /* get filename from a dfits output */
    sscanf(line, "%*s %*s %s", word) ;
    return ;
}
    
/*----------------------------------------------------------------------------
   Function :   isdetectedkeyword()
   In       :   FITS line, set of keywords, number of kw in the set.
   Out      :   keyword rank, -1 if unidentified    
   Job      :   detects a if a keyword is present in a FITS line.
   Notice   :
                Feed this function a FITS line, a set of keywords in the
                *argv[] fashion (*keywords[]).
                If the provided line appears to contain one of the keywords
                registered in the list, the rank of the keyword in the list
                is returned, otherwise, -1 is returned.
 ---------------------------------------------------------------------------*/
static int 
isdetectedkeyword(  char *line, 
                    char *keywords[], 
                    int  nkeys)
{
    int     i ;
    char    kw[MAX_STRING] ;
    char    esokw[MAX_STRING] ;

    /*
     * The keywors is defined as the input line, up to the equal character,
     * with trailing blanks removed
     */
    strcpy(kw, line) ;
    strtok(kw, "=") ;
    /* Now remove all trailing blanks (if any) */
    i = (int)strlen(kw) -1 ;
    while (kw[i] == ' ') i -- ;
    kw[i+1] = (char)0 ;
    
    /* Now compare what we got with what's available */
    for (i=0 ; i<nkeys ; i++) {
		if (strstr(keywords[i], ".")!=NULL) {
			/*
			 * keyword contains a dot, it is a hierarchical keyword that
			 * must be expanded. Pattern is:
			 * A.B.C... becomes HIERARCH ESO A B C ...
			 */
			expand_hierarch_keyword(keywords[i], esokw) ;
			if (!strcmp(kw, esokw)) {
					return i ;
			}
		} else if (!strcmp(kw, keywords[i])) {
            return i ;
        }
    }
    /* Keyword not found    */
    return -1 ;
}


/*---------------------------------------------------------------------------
   Function	:	expand_hierarch_keyword()
   In 		:	two allocated strings
   Out 		:	char *, pointer to second input string (modified)
   Job		:	from a HIERARCH keyword in format A.B.C expand to
   				HIERARCH ESO A B C
   Notice	:
 ---------------------------------------------------------------------------*/

static char *
expand_hierarch_keyword(
	char	*	dotkey,
	char	*	hierarchy
)
{
	char *  token ;
	char    ws[MAX_STRING] ;

	sprintf(hierarchy, "HIERARCH ESO");
	strcpy(ws, dotkey) ;
	token = strtok(ws, ".") ;
	while (token!=NULL) {
		strcat(hierarchy, " ") ;
		strcat(hierarchy, token) ;
		token = strtok(NULL, ".");
	}
	return hierarchy ;
}




/*----------------------------------------------------------------------------
   Function :   getkeywordvalue()
   In       :   FITS line to process, char string to return result
   Out      :   void, result returned in char *word
   Job      :   Get a keyword value within a FITS line
   Notice   :   No complex value is recognized
 ---------------------------------------------------------------------------*/

static void
getkeywordvalue(char *line, char *word)
{
    int     c, w ;
    char    tmp[MAX_STRING] ;
    char    *begin, *end ;
    int     length ;
    int     quote = 0 ;
    int     search = 1 ;

    memset(tmp, (char)0, MAX_STRING) ;
    memset(word, (char)0, MAX_STRING) ;
    c = w = 0;

    /* Parse the line till the equal '=' sign is found  */

    while (line[c] != '=') c++ ;
    c++ ;

    /* Copy the line till the slash '/' sign is found   */
    /* or the end of data is found.                     */

    while (search == 1) {
        
        if (c>=80) {
            search = 0 ;
        } else if ((line[c] == '/') && (quote == 0)) {
            search = 0 ;
        } 

        if (line[c] == '\'') {
            quote = !quote ;
        } 
        
        tmp[w++] = line[c++] ;
    }
    
    /* NULL termination of the string   */
    tmp[--w] = (char)0 ;

    /* Return the keyword only : a difference is made between text fields   */
    /* and numbers.                                                         */

    if ((begin = strchr(tmp, '\'')) != (char*)NULL) {
    /* A quote has been found: it is a string value */
        begin++ ;
        end = strrchr(tmp, '\'') ;
        length = (int)strlen(begin) - (int)strlen(end) ;
        strncpy(word, begin, length) ;
    } else {
    /* No quote, just get the value (only one, no complex supported) */
        sscanf(tmp, "%s", word) ;
    }
        
    return ;
}

