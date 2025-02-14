#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
/*#include <fcntl.h>*/

#ifdef unix
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#else
/*# include <direct.h>*/
/*# include <io.h>*/
#endif

#include "zip.h"

#define MIN(a,b) (a <= b ? a : b)

#define endmark "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define SKIP_MASK ((int) 0x1F)
#define max_members 50000

static long   member_count;
static char * members;

static long readdir (char * pds) {
    long   i;
    long   j;
    FILE * fh;
    char   line [256];
    char   tstr [9];
    char * a;
    char * name;
    short  b;
    short  count;
    short  skip;
    long   quit;
    int    info_byte;
    short  l;

    member_count = 0;

#if 0
    fh = fopen (pds, "rb,klen=0,lrecl=256,blksize=256,recfm=u,force");
#else
    fh = fopen (pds, "rb");
#endif

    if (fh == NULL) {
        printf ("Error opening directory in %s\n", pds);
        return (1);
    };

#if 0
    fread (&l, 1, 2, fh); /* Skip U length */
#endif

    quit = 0;
    while (fread (line, 1, 256, fh) == 256) {

        a = &(line [2]);
        b = ((short *)&(line [0])) [0];
        count = 2;
        while (count < b) {
            
            if (memcmp (a, endmark, 8) == 0) {
                quit = 1;
                break;
            };

            name = a;
            a += 8;

            i = (((int *)a) [0]) & 0xFFFFFF00; /* TTR not used here! */
            a += 3;

            info_byte = (int)(*a);
            a++;

            skip = (info_byte & SKIP_MASK) * 2;
            
            strncpy (tstr, name, 8);
            j = 7;
            while (tstr [j] == ' ') j--;
            tstr [++j] = 0;

            strcpy (&(members [9 * (member_count++)]), tstr);

            if (member_count == max_members) {
                quit = 1;
                break;
            };

            a += skip;

            count += (8 + 4 + skip);
        };

        if (quit) break;

#if 0
        fread (&l, 1, 2, fh); /* Skip U length */
#endif
    };

    fclose (fh);
    return (0);
};

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef XXX_WIN32
uLong filetime(f, tmzip, dt)
    char *f;                /* name of file to get info on */
    tm_zip *tmzip;     /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATA  ff32;

      hFind = FindFirstFile(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
#ifdef unix
uLong filetime(f, tmzip, dt)
    char *f;                /* name of file to get info on */
    tm_zip *tmzip;     /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;
  
  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME];
    int len = strlen(f);
    strcpy(name, f);
    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(f, tmzip, dt)
    char *f;                /* name of file to get info on */
    tm_zip *tmzip;     /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
    return 0;
}
#endif
#endif

void myputval (char * d, long s) {

    d [0] = (s & 0xFF);
    d [1] = ((s >> 8) & 0xFF);
    d [2] = ((s >> 16) & 0xFF);
    d [3] = ((s >> 24) & 0xFF);
};

int check_exist_file(filename)
    const char* filename;
{
	FILE* ftestexist;
    int ret = 1;
	ftestexist = fopen(filename,"rb");
	if (ftestexist==NULL)
        ret = 0;
    else
        fclose(ftestexist);
    return ret;
}

void do_banner()
{
	printf("MiniZip 0.15 MVS 4.0\n");
    printf("Demo of zLib + Zip package written by Gilles Vollant\n");
	printf("more info at http://www.winimage.com/zLibDll/minizip.html\n");
	printf("Modified for MVS, see http://gccmvs.sourceforge.net\n\n");
}
------------------------------------------------------------------------------
void do_help()
{	
	printf("Usage : minizip -abco zipfile files_to_add\n") ;
    printf("-a opens files_to_add in text-translated mode and converts "
        "EBCDIC to ASCII.\n");
    printf("-b zips files without length indicators (use with V,VB or U "
        "datasets only.)\n");
    printf("-c chooses the alternate code-page 037 instead of the default "
        "1047.\n");
    printf("-o specifies that all files_to_add are Partition Organised "
        "datasets and\n");
    printf("that all members/alias's in each dataset should be zipped.\n");
    printf("-l to lowercase names\n");
    printf("-x <extension> to add an extension to all filenames\n");
    printf("SYSUT1 and zipfile need to be allocated as F/FB with any LRECL "
        "and BLKSIZE.\n\n");
}

FILE        * t1fh;
FILE        * fout;
unsigned long g_ftell;
unsigned long g_crc32;
unsigned long g_csize;
unsigned long g_usize;
unsigned long g_offset;

extern void etoa (char * in, long len);
extern unsigned char * __ebcdic_to_ascii;
extern unsigned char   __ebcdic_to_ascii2 [];

#if 0
extern int _vmode;
#endif

int main(argc,argv)
	int argc;
	char *argv[];
{
	int i;
    int j;
    int k;
    int opt_ascii=0;
    int opt_binary=0;
    int opt_lower=0;
    char *ext = "";
    short binary_read;
	int opt_organised=0;
    int opt_compress_level=Z_DEFAULT_COMPRESSION;
    int zipfilenamearg = 0;
	char filename_try[MAXFILENAME];
    int zipok;
    int err=0;
    int size_buf=0;
    void* buf=NULL;

    long glen1, glen2, rlen;
    char * buf1;
    char * buf2;
    int    cmember;
    char   comment [80];
    unsigned char  recfm;
    unsigned short lrecl;
    unsigned short blksize;

    long addfiles = 0;

    char tempfile [64] = "DD:SYSUT1";

	do_banner();

    if (argc==1)
	{
		do_help();
		exit(0);
        return 0;
	}
	else
	{
		for (i=1;i<argc;i++)
		{
			if ((*argv[i])=='-')
			{
				const char *p=argv[i]+1;
				
				while ((*p)!='\0')
				{			
					char c=*(p++);

					if ((c=='o') || (c=='O'))
						opt_organised = 1;
                    else
                    if ((c=='a') || (c=='A')) {
						opt_ascii = 1;
                        opt_binary = 0;
#if 0
                        _vmode = 0;
#endif
                    }
                    else
                    if ((c=='l') || (c=='L')) {
                        opt_lower = 1;
                    }
                    else
                    if ((c=='x') || (c=='X')) {
                        if (argv[++i] != NULL) {
                            ext = argv[i];
                        }
                    }
                    else
                    if ((c=='b') || (c=='B')) {
						opt_ascii = 0;
                        opt_binary = 1;
#if 0
        _vmode = 1; /* switch JCC into 'true binary' mode for V[B] datasets */
#endif
                    }
                    else
                    if ((c=='c') || (c=='C')) {
                        __ebcdic_to_ascii = __ebcdic_to_ascii2;
                    }
                    else
                    if ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    else {
                        printf ("Invalid parameter \"%c\" passed to "
                                "minizip, exiting...\n", c);
                        exit (EXIT_FAILURE);
                    };
				}
			}
			else
				if (zipfilenamearg == 0)
                    zipfilenamearg = i ;
                else
                    addfiles++;
		}
	}

    if (addfiles == 0) {
        printf ("There were no specified files to add, exiting...\n");
        exit (EXIT_FAILURE);
    };

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    buf2 = (char *)malloc(3200);
    members = (char *)malloc(9 * max_members);
    if ((buf==NULL) || (buf2 == NULL) || (members == NULL))
    {
        printf("Error allocating memory\n");
        return ZIP_INTERNALERROR;
    }

	if (zipfilenamearg==0)
        zipok=0;
    else
	{
        zipok = 1 ;
		strcpy(filename_try,argv[zipfilenamearg]);
    }

    if (zipok==1)
    {
        zipFile zf;
        int errclose;

        g_offset = 0;

        zf = zipOpen(filename_try,0);
        if (zf == NULL)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        else 
            printf("creating %s\n",filename_try);

        for (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            if (((*(argv[i]))!='-') /*&& ((*(argv[i]))!='/')*/)
            {
                FILE * fin;
                int size_read;
                zip_fileinfo zi;
            	char filenameinzip [MAXFILENAME];
            	char filememberinzip [MAXFILENAME];
                char * ss;
                int j;

                if (opt_organised) {
                    readdir (argv[i]);
                    /* Don't process an empty dataset */
                    if (member_count == 0) continue; 
                };
                cmember = 0;
                do {
                    if (opt_organised)
                        sprintf (filenameinzip, "%s(%s)", 
                                 argv[i], &(members [9 * cmember++]));
                    else
                        strcpy (filenameinzip, argv[i]);

                    /* Only store the member-name (if one exists...) */
                    if ((ss = strstr (filenameinzip, "(")) != NULL) {
                        ss++;
                        j = 0;
                        while ((*ss) && (*ss != ')'))
                            filememberinzip [j++] = *ss++;
                        filememberinzip [j++] = 0;
                        strcat(filememberinzip, ext);
                        if (opt_lower) {
                          for (j = 0; filememberinzip[j] != 0; j++) {
                            filememberinzip[j] = tolower(filememberinzip[j]);
                          }
                        }
                    } else
                    if ((filenameinzip [0] == '/') && /* No member, then: */
                        (filenameinzip [1] == '/') && 
                        /* Remove "DD:" or "//DSN:" */
                        (toupper(filenameinzip [2]) == 'D') &&
                        (filenameinzip [3] != 0) &&
                        (toupper(filenameinzip [4]) == 'N') &&
                        (filenameinzip [5] == ':'))
                        strcpy (filememberinzip, &(filenameinzip [6]));
                    else if (
                        (toupper(filenameinzip [0]) == 'D') &&
                        (toupper(filenameinzip [1]) == 'D') &&
                        (filenameinzip [2] == ':'))
                        strcpy (filememberinzip, &(filenameinzip [3]));
                    else
                        strcpy (filememberinzip, filenameinzip); /* fallback! */

                    zi.tmz_date.tm_sec = 
                    zi.tmz_date.tm_min = 
                    zi.tmz_date.tm_hour = 
                    zi.tmz_date.tm_mday = 
                    zi.tmz_date.tm_min = 
                    zi.tmz_date.tm_year = 0;
                    zi.dosDate = 0;
/*                    zi.internal_fa = 0;*/
                    if (opt_ascii) {
                        zi.internal_fa = 1;
                    } else {
                        zi.internal_fa = 2;
                    };
                    zi.external_fa = 0;
                    filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

#if 0
                    t1fh = fopen (tempfile, "wb,umode=0");
#else
                    t1fh = fopen (tempfile, "wb");
#endif
                    if (t1fh == NULL) {
                        
                        if (tempfile [3] != 'S') { /* //DDN: */
                            printf ("Warning: can't open %s\n", tempfile);

                            strcpy (tempfile, "//DSN:&&TMPZIP");
                            t1fh = fopen (tempfile, "wb,umode=0,recfm=fb,"
                                "lrecl=80,blksize=3120,unit=sysda,pri=150");
                        };

                        if (t1fh == NULL) {
                            printf ("Fatal error: can't open %s\n", tempfile);
                            exit (EXIT_FAILURE);
                        };
                    };

                    if (opt_ascii)
                        fin = fopen(filenameinzip,"r");
                    else
                        fin = fopen(filenameinzip,"rb");
                    if (fin==NULL)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading (rc:%d, %d)\n",
                            filenameinzip, err, errno);
                    }

                    if (err == ZIP_OK) {

#if 0
             if (__getdcb (_fileno(fin), 
                    NULL, &recfm, NULL, 
                    &lrecl, &blksize) == 0) {
                            if (lrecl == 0) lrecl = blksize;
                            switch (recfm) {
                            case 0x40:
                                sprintf (comment, " ,V%d", lrecl);
                                break;
                            case 0x50:
                                sprintf (comment, " ,VB%d", lrecl);
                                break;
                            case 0x80:
                                sprintf (comment, " ,F%d", lrecl);
                                break;
                            case 0x90:
                                sprintf (comment, " ,FB%d", lrecl);
                                break;
                            case 0xC0:
                                sprintf (comment, " ,U%d", blksize);
                                break;
                            default:
                                comment [0] = 0;
                            };
                            if (comment [0]) {
                                if (opt_ascii) {
                                    comment [0] = 'A';
                                } else {
                                    if (opt_binary) {
                                        comment [0] = 'B';
                                    } else {
                                        comment [0] = 'E';
                                    };
                                };
                            };
                        } else {
#else
{
#endif
                            comment [0] = 0;
                        };

                        err = zipOpenNewFileInZip(zf,filememberinzip,&zi,
                                NULL,0,NULL,0,/*NULL*/ comment,
                                (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                opt_compress_level);

                        glen1 = ftell (t1fh);
                        fclose (t1fh);

#if 0
                        t1fh = fopen (tempfile, "rb,umode=0");
#else
                        t1fh = fopen (tempfile, "rb");
#endif
                        if (t1fh == NULL) {
                            printf ("Fatal error: can't reopen %s\n"
                                , tempfile);
                            exit (EXIT_FAILURE);
                        };

                        buf1 = malloc (glen1);
                        if (buf1 == NULL) {
                            printf ("Fatal error: can't allocate memory\n");
                            exit (EXIT_FAILURE);
                        };

                        fread (buf1, 1, glen1, t1fh);

                        fclose (t1fh);

#if 0
                        t1fh = fopen (tempfile, "wb,umode=0");
#else
                        t1fh = fopen (tempfile, "wb");
#endif
                        if (t1fh == NULL) {
                            printf ("Fatal error: can't open %s\n", tempfile);
                            exit (EXIT_FAILURE);
                        };

                        if (err != ZIP_OK)
                            printf("error in opening %s in zipfile"
                                " (rc:%d, %d)\n",filenameinzip, err, errno);
                        /* Nothing in the read buffer yet - V[B] and U only */
                        binary_read = 0; 
                    };

                    if (err == ZIP_OK)
                        do
                        {
                            err = ZIP_OK;
                            if (opt_binary) {
                                
                                if (binary_read) { 
                                    /* still data in the current buffer... */
                                    size_read = fread(buf,1,MIN(size_buf,
                                                      binary_read),fin);
                                    binary_read -= size_read;
                                    k = size_buf - size_read;
                                } else {
                                    size_read = 0;
                                    k = size_buf;
                                };

                                while (size_read < size_buf) {
                                    if (fread (&binary_read,1,2,fin) != 2) {
                                        binary_read = 0;
                                        break;
                                    };
                                    j = fread(&(((char *)buf) [size_read]),1,
                                              MIN(k,binary_read),fin);
                                    size_read += j;
                                    binary_read -= j;
                                    k -= j;
                                };

                            } else
                                size_read = fread(buf,1,size_buf,fin);

                            if (size_read < size_buf)
                                if (feof(fin)==0)
                            {
                                printf("error in reading %s\n",filenameinzip);
                                err = ZIP_ERRNO;
                            }

                            if (size_read>0)
                            {
                                if (opt_ascii)
                                    etoa (buf,size_read);

                                err = zipWriteInFileInZip (zf,buf,size_read);
                                if (err<0)
                                {
                                    printf("error in writing %s in the "
                                           "zipfile (rc:%d, %d)\n",
                                             filenameinzip, err, errno);
                                }
                                
                            }
                        } while ((err == ZIP_OK) && (size_read>0));

                    if (fin!=NULL)
                        fclose(fin);

                    if (err<0)
                        err=ZIP_ERRNO;
                    else
                    {                    
                        err = zipCloseFileInZip(zf);
                        if (err!=ZIP_OK) {
                    
                            printf("error in closing %s in the "
                                    "zipfile (rc:%d, %d)\n",
                                        filenameinzip, err, errno);

                        } else { /* Merge SYSUT files now... */

                            myputval (&(buf1 [g_ftell + 0]), g_crc32);
                            myputval (&(buf1 [g_ftell + 4]), g_csize);
                            myputval (&(buf1 [g_ftell + 8]), g_usize);
            
                            fwrite (buf1, 1, glen1, fout);

                            glen2 = ftell (t1fh);
                            fclose (t1fh);

#if 0
                            t1fh = fopen (tempfile, "rb,umode=0");
#else
                            t1fh = fopen (tempfile, "rb");
#endif
                            if (t1fh == NULL) {
                                printf ("Fatal error: can't"" reopen %s\n",
                                        tempfile);
                                exit (EXIT_FAILURE);
                            };

                            do {
                                if (glen2 > 3200)
                                    rlen = 3200;
                                else
                                    rlen = glen2;

                                rlen = fread (buf2, 1, rlen, t1fh);
                                fwrite (buf2, 1, rlen, fout);

                                glen2 -= rlen;

                            } while (rlen == 3200);

                            g_offset = ftell (fout);
                        };
                    }

                    free (buf1);
                    fclose (t1fh);
                } while ((opt_organised) && (cmember < member_count));
            }
        }
        errclose = zipClose(zf,NULL);
        if (errclose != ZIP_OK)
            printf("error in closing %s (rc:%d, %d)\n",filename_try, 
                    errclose, errno);
   }

    free(buf);
    free(buf2);
    free(members);

    return 0;
}
