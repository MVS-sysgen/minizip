#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/*#include <fcntl.h>*/

#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
/*# include <direct.h>*/
/*# include <io.h>*/
#endif

#include "unzip.h"

/* make external names 8-chars unique */
#define do_extract_currentfile decurfil
#define do_extract_onefile deonefil

#define CASESENSITIVITY (2) /* no sensitivity to input filenames */
#define WRITEBUFFERSIZE (8192)

/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract]

  list the file in the zipfile, and print the 
  content of FILE_ID.ZIP or README.TXT if it exists
*/


void do_banner()
{
	printf("MiniUnz 0.15 MVS 4.0\n");
	printf("Demo of zLib + Unz package written by Gilles Vollant\n");
	printf("more info at http://www.winimage.com/zLibDll/minizip.html\n");
    printf("Modified for MVS - see http://gccmvs.sourceforge.net\n\n");
}

extern void atoe (char * in, long len);
extern unsigned char * __ascii_to_ebcdic;
extern unsigned char   __ascii_to_ebcdic2 [];

int opt_ascii=0;
static int verbose = 0;

void do_help()
{	
	printf("Usage : miniunz -aclv zipfile dest_file file_to_extract\n") ;
    printf("-a opens files in text-translated mode and converts ASCII"
		   " to EBCDIC.\n");
    printf("-c chooses the alternate code-page 037 instead of the default"
		   " 1047.\n");
    printf("-l or -v only lists statistics and files in the zip archive.\n");
    printf("If no file_to_extract is specified, all files are extracted "
		   "and\n");
    printf("the destination file will have (member) automatically "
		   "appended.\n\n");
}

int do_list(uf)
	unzFile uf;
{
	uLong i;
	unz_global_info gi;
	int err;
    char comment [80];

	err = unzGetGlobalInfo (uf,&gi);
	if (err!=UNZ_OK)
		printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    printf(" Length  Method   Size  Ratio   Date    Time   CRC-32     Name\n");
    printf(" ------  ------   ----  -----   ----    ----   ------     ----\n");
	for (i=0;i<gi.number_entry;i++)
	{
		char filename_inzip[256];
		unz_file_info file_info;
		uLong ratio=0;
		const char *string_method;

        err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,
			                        sizeof(filename_inzip),NULL,0,
									&(comment [2]),75);

        if ((verbose) && (comment [2])) {
            comment [0] = ' ';
            comment [1] = '{';
            strcat (comment, "}");
        } else {
            comment [0] = 0;
        };

		if (err!=UNZ_OK)
		{
			printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
			break;
		}
		if (file_info.uncompressed_size>0)
			ratio = (file_info.compressed_size*100)/file_info.uncompressed_size;

		if (file_info.compression_method==0)
			string_method="Stored";
		else
		if (file_info.compression_method==Z_DEFLATED)
		{
			uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
			if (iLevel==0)
			  string_method="Defl:N";
			else if (iLevel==1)
			  string_method="Defl:X";
			else if ((iLevel==2) || (iLevel==3))
			  string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
		}
		else
        if (file_info.compression_method==1)
			string_method="Shrunk";
        else
			string_method="Unkn. ";

		printf("%7lu  %6s %7lu %3lu%%  %2.2lu-%2.2lu-%2.2lu "
			   " %2.2lu:%2.2lu  %8.8lx   %s%s\n",
			    file_info.uncompressed_size,
				string_method,
				file_info.compressed_size,
				ratio,
				(uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
				(uLong)file_info.tmu_date.tm_year % 100,
				(uLong)file_info.tmu_date.tm_hour,
				(uLong)file_info.tmu_date.tm_min,
				(uLong)file_info.crc,filename_inzip, comment);
		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err!=UNZ_OK)
			{
				printf("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	return 0;
}


/* replace characters that JCL can't handle */

static char *fix_mvs_member(char *p)
{
    while (strchr(p, '-') != NULL) *strchr(p, '-') = '@';
    while (strchr(p, '_') != NULL) *strchr(p, '_') = '@';
    if (strlen(p) > 8)
    {
        p[8] = '\0';
    }
    return (p);
}


/*****************************************************************/
/***************************** UNSHRINK **************************/
/*****************************************************************/

#include "unshr.h"

min_info   unsM;
Uz_Globs * unsG;

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

int do_extract_currentfile(uf,
						   popt_extract_without_path,
						   popt_overwrite,
						   filename_to_write,
						   member)
	unzFile uf;
	const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* filename_to_write; /* Where to write output */
    int         member; /* Does output file need (member) appended? */
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;
    int i;
    int j;
    int k;
    int l;
    char * s;
    char * t;
	
	unz_file_info file_info;
	uLong ratio=0;
	
    char filename_dst[512];

    err = unzGetCurrentFileInfo(uf,
								&file_info,
								filename_inzip,
								sizeof(filename_inzip),
								NULL,0,NULL,0);

	if (err!=UNZ_OK)
	{
		printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf + 1);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0')
	{
		if (((*p)=='/') || ((*p)=='\\'))
			filename_withoutpath = p+1;
		p++;
	}

	if ((*filename_withoutpath)=='\0') /* do nothing! */
	{
/*		if ((*popt_extract_without_path)==0)
		{
			printf("creating directory: %s\n",filename_inzip);
			mymkdir(filename_inzip);
		}
*/	}
	else
	{
		const char* write_filename;
		int skip=0;

        if (member) {
            p = strstr (filename_withoutpath, ".");
            if ((p) && (p != filename_withoutpath)) p [0] = 0;
            fix_mvs_member(filename_withoutpath);
            sprintf (filename_dst, "%s(%s)", filename_to_write, 
					 filename_withoutpath);
            write_filename = filename_dst;
        } else
#ifdef __CMS__
            write_filename = filename_withoutpath;
#else
            write_filename = filename_to_write;
#endif
/*
		if ((*popt_extract_without_path)==0)
			write_filename = filename_inzip;
		else
			write_filename = filename_withoutpath;
*/
		err = unzOpenCurrentFile(uf);
		if (err!=UNZ_OK)
		{
			printf("error %d with zipfile in unzOpenCurrentFile\n",err);
		}

		if ((skip==0) && (err==UNZ_OK))
		{
            if (opt_ascii)
			    fout=fopen(write_filename,"w");
            else
			    fout=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */

            /* BUT WE DON'T CARE!!!
            if ((fout==NULL) && ((*popt_extract_without_path)==0) && 
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }
            */
			if (fout==NULL)
			{
				printf("error opening %s\n",write_filename);
			}
		}

		if (fout!=NULL)
		{
            /* Get ready for unshrink (if it's needed) */
            unsG->outfd = fout;
            unsG->pInfo = &unsM;
            unsG->pInfo->textmode = 0;
            unsG->zipeof = 0;
            unsG->bits_left = 0;

			printf(" extracting: %s\n",write_filename);

			do
			{
				err = unzReadCurrentFile(uf,buf,size_buf);
				if (err<0)	
				{
					printf("error %d with zipfile in"
						   " unzReadCurrentFile\n",err);
					break;
				}
                if (err>0) {
                    
                    l = err;
                    if (opt_ascii) { /* Convert buffer */
                        t = buf;
                        atoe (t, l);
                        if (t [l-1] == '\r') l--;
                        i = 0;
                        t [l] = 0; /* assume no embedded nulls */
                        k = l;
                        s = strstr (t, "\r\n"); /* any to replace? */
                        while (s) {
							/* make CR - LF, but don't write the second one */
                            s [0] = '\n'; 
							/* calculate the right length */
                            j = (s - t) + 1;   
                            i += fwrite (t, 1, j, fout);
							/* make up for the missing LF now */
                            i++;          

                            k -= ((s - t) + 2);
							/* just past the two control characters... */
                            t = s + 2;        
                            s = strstr (t, "\r\n");
                        };
                        i += fwrite (t, 1, k, fout);

                    } else {
                        i = fwrite(buf,1,l,fout);
                    };

					if (i!=l)
					{
						printf("error in writing extracted "
							   "file (%d#%d)\n", i, l);
                        err=UNZ_ERRNO;
						break;
					}
                };
			}
			while (err>0);
			fclose(fout);
		}

        if (err==UNZ_OK)
        {
		    err = unzCloseCurrentFile (uf);
		    if (err!=UNZ_OK)
		    {
			    printf("error %d with zipfile in unzCloseCurrentFile\n",err);
		    }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */       
	}

    free(buf);    
    return err;
}


int do_extract(uf,opt_extract_without_path,opt_overwrite,filename_to_write)
	unzFile uf;
	int opt_extract_without_path;
    int opt_overwrite;
    const char* filename_to_write;
{
	uLong i;
	unz_global_info gi;
	int err;
	FILE* fout=NULL;

	err = unzGetGlobalInfo (uf,&gi);
	if (err!=UNZ_OK)
		printf("error %d with zipfile in unzGetGlobalInfo \n",err);

	for (i=0;i<gi.number_entry;i++)
	{
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,filename_to_write,
#ifdef __MVS__
                                      1
#else
                                      0
#endif
				      ) != UNZ_OK)
            break;

		if ((i+1)<gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err!=UNZ_OK)
			{
				printf("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	return 0;
}

int do_extract_onefile(uf,
					   filename,
					   opt_extract_without_path,
					   opt_overwrite,
					   filename_to_write)
	unzFile uf;
	const char* filename;
	int opt_extract_without_path;
    int opt_overwrite;
    const char* filename_to_write;
{
    int err = UNZ_OK;
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,
							   &opt_extract_without_path,
                               &opt_overwrite,
							   filename_to_write,0) == UNZ_OK)
        return 0;
    else
        return 1;
}


int main(argc,argv)
	int argc;
	char *argv[];
{
	const char *zipfilename=NULL;
    const char *filename_to_extract=NULL;
    const char *filename_to_write=NULL;
	int i;
	int opt_do_list=0;
	int opt_do_extract=1;             /* if not listing, extract */
	int opt_do_extract_withoutpath=1; /* always remove path */
	int opt_overwrite=1;              /* always overwrite files */
	unzFile uf=NULL;

	do_banner();
	if (argc==1)
	{
		do_help();
		exit(0);
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

					if ((c=='a') || (c=='A'))
						opt_ascii = 1;
                    else
                    if ((c=='c') || (c=='C'))
                        __ascii_to_ebcdic = __ascii_to_ebcdic2;
                    else
					if ((c=='l') || (c=='L'))
						opt_do_list = 1;
                    else
                    if ((c=='v') || (c=='V')) {
						opt_do_list = 1;
                        verbose = 1;
                    }
                    else {
                        printf ("Invalid parameter \"%c\" passed to"
							    " miniunz, exiting...\n", c);
                        exit (EXIT_FAILURE);
                    };
				}
			}
			else
            {
				if (zipfilename == NULL)
					zipfilename = argv[i];
                else
                if (filename_to_write == NULL)
                    filename_to_write = argv [i];
                else
                if (filename_to_extract == NULL)
                    filename_to_extract = argv[i] ;
            }
		}
	}

	if (zipfilename!=NULL)
		uf = unzOpen(zipfilename);

	if (uf==NULL)
	{
		printf("Cannot open %s\n",zipfilename);
		exit (EXIT_FAILURE);
	}
    printf("%s opened\n",zipfilename);

	if (opt_do_list==1)
		return do_list(uf);
	else if ((opt_do_extract==1) 
#ifndef __CMS__
		&& (filename_to_write != NULL)
#endif
		)
    {
        unsG = (Uz_Globs *)malloc (sizeof (Uz_Globs));
        if (unsG != NULL) {

            if (filename_to_extract == NULL)
		        i = do_extract(uf,
							   opt_do_extract_withoutpath,
							   opt_overwrite,
							   filename_to_write);
            else
                i = do_extract_onefile(uf,
			   						   filename_to_extract,
                                       opt_do_extract_withoutpath,
									   opt_overwrite,
									   filename_to_write);

            free (unsG);

            return (i);

        } else {
            printf ("Error allocating unshrink buffers\n");
        };
    }
    else
		printf("No destination file was specified\n");

	unzCloseCurrentFile(uf);

	return 0;  /* to avoid warning */
}
