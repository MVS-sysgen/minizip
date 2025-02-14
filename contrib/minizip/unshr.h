typedef unsigned char   uch;    /* code assumes unsigned bytes; these type-  */
typedef unsigned short  ush;    /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long   ulg;    /*  predefined on some systems) & match zip  */
typedef int             shrint; /* for efficiency/speed, we hope... */

#define MAX_BITS    13                 /* used in unshrink() */
#define HSIZE       (1 << MAX_BITS)    /* size of global work area */
#define WSIZE   0x8000  /* window size--must be a power of two, and */
                        /*  at least 32K for zip's deflate method */
#define lenEOL    1
#define OUTBUFSIZ (lenEOL*WSIZE) /* more efficient text conversion */
#define RAWBUFSIZ OUTBUFSIZ

union work {
   struct {                 /* unshrink(): */
       shrint Parent[HSIZE];    /* (8192 * sizeof(shrint)) == 16KB minimum */
       uch value[HSIZE];        /* 8KB */
       uch Stack[HSIZE];        /* 8KB */
   } shrink;                  /* total = 32KB minimum; 80KB on Cray/Alpha */
};

typedef struct min_info {
    int textmode;   /* file is to be extracted as text */
} min_info;

typedef struct Globals {
    int       inlen;     /* length of read data left */
    int       inmax;     /* max length of buffer */

    void    * ccrc;      /* hack in crc check */

    FILE    * zipfd;     /* zipfile file handle */
    FILE    * outfd;     /* outfile file handle */

    union work area;     /* space to do calculations */
    min_info * pInfo;    /* textmode = 0 - no translation, no additional memory */

    uch     * inbuf;     /* input buffer */
    uch     * inptr;     /* pointer into input buffer, point to inbuf */
    int       incnt;     /* count of bytes in input buffer */

    int       zipeof;    /* temp EOF value for NEXTBYTE, (initially == 0) */

    ulg       bitbuf;    /* 32 bit buffer, uninitialised */
    int       bits_left; /* unreduce and unshrink only, (initially == 0) */

    uch       outbuf [RAWBUFSIZ]; /* output area */
    uch     * outptr;    /* temp pointer to outbuf, uninitialised */
    ulg       outcnt;    /* number of chars stored in outbuf, uninitialised */
} Uz_Globs;
