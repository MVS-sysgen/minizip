/* zip.c -- IO on .zip files using zlib 
   Version 0.15 beta, Mar 19th, 1998,

   Read zip.h for more info
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include "zip.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif


/*******************************************/

/* IBM Code Page 037 -> ISO Latin-1 */

unsigned char __ebcdic_to_ascii2 [] = {
    "\x00\x01\x02\x03\x9C\x09\x86\x7F\x97\x8D\x8E\x0B\x0C\x0D\x0E\x0F"
    "\x10\x11\x12\x13\x9D\x85\x08\x87\x18\x19\x92\x8F\x1C\x1D\x1E\x1F"
    "\x80\x81\x82\x83\x84\x0A\x17\x1B\x88\x89\x8A\x8B\x8C\x05\x06\x07"
    "\x90\x91\x16\x93\x94\x95\x96\x04\x98\x99\x9A\x9B\x14\x15\x9E\x1A"
    "\x20\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\x2E\x3C\x28\x2B\x7C"
    "\x26\xAA\xAB\xAC\xAD\xAE\xAF\xB0\xB1\xB2\x21\x24\x2A\x29\x3B\xD9"
    "\x2D\x2F\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\x2C\x25\x5F\x3E\x3F"
    "\xBC\xBD\xBE\xBF\xC0\xC1\xC2\xC3\xC4\x60\x3A\x23\x40\x27\x3D\x22"
    "\xC5\x61\x62\x63\x64\x65\x66\x67\x68\x69\xC6\xC7\xC8\xC9\xCA\xCB"
    "\xCC\x6A\x6B\x6C\x6D\x6E\x6F\x70\x71\x72\xCD\xCE\xCF\xD0\xD1\xD2"
    "\xD3\x7E\x73\x74\x75\x76\x77\x78\x79\x7A\xD4\xD5\xD6\xE3\xD7\xD8"
    "\x5E\xDA\xDB\xDC\xDD\xDE\xDF\xE0\xE1\xE2\x5B\x5D\xE5\xE4\xE6\xE7"
    "\x7B\x41\x42\x43\x44\x45\x46\x47\x48\x49\xE8\xE9\xEA\xEB\xEC\xED"
    "\x7D\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\xEE\xEF\xF0\xF1\xF2\xF3"
    "\x5C\xF4\x53\x54\x55\x56\x57\x58\x59\x5A\xF5\xF6\xF7\xF8\xF9\xFA"
    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\xFB\xFC\xFD\xFE\xFF\x9F"
};

/* IBM Code Page 1047 -> ISO Latin-1 */

unsigned char __ebcdic_to_ascii1 [] = {
    "\x00\x01\x02\x03\x9C\x09\x86\x7F\x97\x8D\x8E\x0B\x0C\x0D\x0E\x0F"
    "\x10\x11\x12\x13\x9D\x0A\x08\x87\x18\x19\x92\x8F\x1C\x1D\x1E\x1F"
    "\x80\x81\x82\x83\x84\x85\x17\x1B\x88\x89\x8A\x8B\x8C\x05\x06\x07"
    "\x90\x91\x16\x93\x94\x95\x96\x04\x98\x99\x9A\x9B\x14\x15\x9E\x1A"
    "\x20\xA0\xE2\xE4\xE0\xE1\xE3\xE5\xE7\xF1\xA2\x2E\x3C\x28\x2B\x7C"
    "\x26\xE9\xEA\xEB\xE8\xED\xEE\xEF\xEC\xDF\x21\x24\x2A\x29\x3B\x5E"
    "\x2D\x2F\xC2\xC4\xC0\xC1\xC3\xC5\xC7\xD1\xA6\x2C\x25\x5F\x3E\x3F"
    "\xF8\xC9\xCA\xCB\xC8\xCD\xCE\xCF\xCC\x60\x3A\x23\x40\x27\x3D\x22"
    "\xD8\x61\x62\x63\x64\x65\x66\x67\x68\x69\xAB\xBB\xF0\xFD\xFE\xB1"
    "\xB0\x6A\x6B\x6C\x6D\x6E\x6F\x70\x71\x72\xAA\xBA\xE6\xB8\xC6\xA4"
    "\xB5\x7E\x73\x74\x75\x76\x77\x78\x79\x7A\xA1\xBF\xD0\x5B\xDE\xAE"
    "\xAC\xA3\xA5\xB7\xA9\xA7\xB6\xBC\xBD\xBE\xDD\xA8\xAF\x5D\xB4\xD7"
    "\x7B\x41\x42\x43\x44\x45\x46\x47\x48\x49\xAD\xF4\xF6\xF2\xF3\xF5"
    "\x7D\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\xB9\xFB\xFC\xF9\xFA\xFF"
    "\x5C\xF7\x53\x54\x55\x56\x57\x58\x59\x5A\xB2\xD4\xD6\xD2\xD3\xD5"
    "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\xB3\xDB\xDC\xD9\xDA\x9F"
};

unsigned char * __ebcdic_to_ascii = __ebcdic_to_ascii1;

void etoa (char * in, long len) {
    while (len--) {
        *in = __ebcdic_to_ascii [*in];
        in++;
    };
};

extern FILE        * t1fh;
extern FILE        * fout;
extern unsigned long g_ftell;
extern unsigned long g_crc32;
extern unsigned long g_csize;
extern unsigned long g_usize;
extern unsigned long g_offset;

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

/*******************************************/

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0F00) /* platform depedent */
#endif

#ifndef Z_BUFSIZE
#define Z_BUFSIZE (16384)
#endif

#ifndef Z_MAXFILENAMEINZIP
#define Z_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

const char zip_copyright[] =
   " zip 0.15 Copyright 1998 Gilles Vollant ";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

typedef struct linkedlist_datablock_internal_s
{
  struct linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignement */
  unsigned char data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

typedef struct linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


typedef struct
{
	z_stream stream;            /* zLib stream structure for inflate */
    int  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    uLong pos_local_header;     /* offset of the local header of the file 
                                     currenty writing */
    char* central_header;       /* central header data for the current file */
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong flag;                 /* flag of the file currently writing */

    int  method;                /* compression method of file currenty wr.*/
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
} curfile_info;

typedef struct
{
    FILE * filezip;
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    int  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile_info ci;            /* info on the file curretly writing */

    uLong begin_pos;            /* position of the beginning of the zipfile */
    uLong number_entry;
} zip_internal;

local linkedlist_datablock_internal* allocate_new_datablock()
{
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(sizeof(linkedlist_datablock_internal));
    if (ldi!=NULL)
    {
        ldi->next_datablock = NULL ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    return ldi;
}

local void free_datablock(ldi)
    linkedlist_datablock_internal* ldi;
{
    while (ldi!=NULL)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        TRYFREE(ldi);
        ldi = ldinext;
    }
}

local void init_linkedlist(ll)
    linkedlist_data* ll;
{
    ll->first_block = ll->last_block = NULL;
}

local void free_linkedlist(ll)
    linkedlist_data* ll;
{
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = NULL;
}


local int add_data_in_datablock(ll,buf,len)
    linkedlist_data* ll;    
    const void* buf;
    uLong len;
{
    linkedlist_datablock_internal* ldi;
    const unsigned char* from_copy;

    if (ll==NULL)
        return ZIP_INTERNALERROR;

    if (ll->last_block == NULL)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        if (ll->first_block == NULL)
            return ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (unsigned char*)buf;

    while (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned char* to_copy;

        if (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            if (ldi->next_datablock == NULL)
                return ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        if (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        else
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        for (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    return ZIP_OK;
}


local int write_datablock(fout,ll)
    FILE * fout;
    linkedlist_data* ll;    
{
    linkedlist_datablock_internal* ldi;
    ldi = ll->first_block;
    while (ldi!=NULL)
    {
        if (ldi->filled_in_this_block > 0)
            if (fwrite(ldi->data,(uInt)ldi->filled_in_this_block,1,fout)!=1)
                return ZIP_ERRNO;
        ldi = ldi->next_datablock;
    }
    return ZIP_OK;
}

/****************************************************************************/

/* ===========================================================================
   Outputs a long in LSB order to the given file
   nbByte == 1, 2 or 4 (byte, short or long)
*/

local int ziplocal_putValue OF((FILE *file, uLong x, int nbByte));
local int ziplocal_putValue (file, x, nbByte)
    FILE *file;
    uLong x;
    int nbByte;
{
    unsigned char buf[4];
    int n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned char)(x & 0xff);
        x >>= 8;
    }
    if (fwrite(buf,nbByte,1,file)!=1)
        return ZIP_ERRNO;
    else
        return ZIP_OK;
}

local void ziplocal_putValue_inmemory OF((void* dest, uLong x, int nbByte));
local void ziplocal_putValue_inmemory (dest, x, nbByte)
    void* dest;
    uLong x;
    int nbByte;
{
    unsigned char* buf=(unsigned char*)dest;
    int n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned char)(x & 0xff);
        x >>= 8;
    }
}
/****************************************************************************/


local uLong ziplocal_TmzDateToDosDate(ptm,dosDate)
    tm_zip* ptm;
    uLong dosDate;
{
    uLong year = (uLong)ptm->tm_year;
    if (year>1980)
        year-=1980;
    else if (year>80)
        year-=80;
    return
      (uLong) (((ptm->tm_mday) + (32 * (ptm->tm_mon+1)) + (512 * year)) << 16) |
        ((ptm->tm_sec/2) + (32* ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

extern zipFile ZEXPORT zipOpen (pathname, append)
    const char *pathname;
    int append;
{
    zip_internal ziinit;
    zip_internal* zi;

    ziinit.filezip = fopen(pathname,(append == 0) ? "wb" : "ab");
#endif
    if (ziinit.filezip == NULL)
        return NULL;
    ziinit.begin_pos = ftell(ziinit.filezip);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.number_entry = 0;
    init_linkedlist(&(ziinit.central_dir));


    zi = (zip_internal*)ALLOC(sizeof(zip_internal));
    if (zi==NULL)
    {
        fclose(ziinit.filezip);
        return NULL;
    }

    *zi = ziinit;

    return (zipFile)zi;
}

extern int ZEXPORT zipOpenNewFileInZip (file, 
                                        filename,
                                        zipfi, 
                                        extrafield_local,
                                        size_extrafield_local,
                                        extrafield_global,
                                        size_extrafield_global,
                                        comment,
                                        method,
                                        level)
    zipFile file;
    const char* filename;
    const zip_fileinfo* zipfi;
    const void* extrafield_local;
    uInt size_extrafield_local;
    const void* extrafield_global;
    uInt size_extrafield_global;
    const char* comment;
    int method;
    int level;
{
    zip_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    int err = ZIP_OK;
    char tstr [260];
    FILE * tfh;

    if (file == NULL)
        return ZIP_PARAMERROR;
    if ((method!=0) && (method!=Z_DEFLATED))
        return ZIP_PARAMERROR;

    zi = (zip_internal*)file;

    fout = zi->filezip;
    tfh = zi->filezip;
    zi->filezip = t1fh;

    if (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        if (err != ZIP_OK) {
            zi->filezip = tfh;
            return err;
        };
    }


    if (filename==NULL)
        filename="-";

    if (comment==NULL)
        size_comment = 0;
    else
        size_comment = strlen(comment);

    size_filename = strlen(filename);

    if (zipfi == NULL)
        zi->ci.dosDate = 0;
    else
    {
        if (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        else zi->ci.dosDate = ziplocal_TmzDateToDosDate(&zipfi->tmz_date,
                                                        zipfi->dosDate);
    }

    zi->ci.flag = 0;
    if ((level==8) || (level==9))
      zi->ci.flag |= 2;
    if ((level==2))
      zi->ci.flag |= 4;
    if ((level==1))
      zi->ci.flag |= 6;

    zi->ci.crc32 = 0;
    zi->ci.method = method;
    zi->ci.stream_initialised = 0;
    zi->ci.pos_in_buffered_data = 0;
    zi->ci.pos_local_header = ftell(zi->filezip) + g_offset;
    zi->ci.size_centralheader = SIZECENTRALHEADER + size_filename + 
                                      size_extrafield_global + size_comment;
    zi->ci.central_header = (char*)ALLOC((uInt)zi->ci.size_centralheader);

    ziplocal_putValue_inmemory(zi->ci.central_header,
                                (uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    ziplocal_putValue_inmemory(zi->ci.central_header+4,(uLong)VERSIONMADEBY,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+6,(uLong)20,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+10,
                                (uLong)zi->ci.method,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+12,
                                (uLong)zi->ci.dosDate,4);
    ziplocal_putValue_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    /*compr size*/
    ziplocal_putValue_inmemory(zi->ci.central_header+20,(uLong)0,4);
    /*uncompr size*/ 
    ziplocal_putValue_inmemory(zi->ci.central_header+24,(uLong)0,4);
    ziplocal_putValue_inmemory(zi->ci.central_header+28,
                                (uLong)size_filename,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+30,
                                (uLong)size_extrafield_global,2);
    ziplocal_putValue_inmemory(zi->ci.central_header+32,
                                (uLong)size_comment,2);
    /*disk nm start*/
    ziplocal_putValue_inmemory(zi->ci.central_header+34,(uLong)0,2); 

    if (zipfi==NULL)
        ziplocal_putValue_inmemory(zi->ci.central_header+36,(uLong)0,2); 
    else
        ziplocal_putValue_inmemory(zi->ci.central_header+36,
                                    (uLong)zipfi->internal_fa,2); 

    if (zipfi==NULL)
        ziplocal_putValue_inmemory(zi->ci.central_header+38,(uLong)0,4); 
    else
        ziplocal_putValue_inmemory(zi->ci.central_header+38,
                                    (uLong)zipfi->external_fa,4);

    ziplocal_putValue_inmemory(zi->ci.central_header+42,
                                (uLong)zi->ci.pos_local_header,4);

    for (i=0;i<size_filename;i++)
        *(zi->ci.central_header + SIZECENTRALHEADER + i) =
             __ebcdic_to_ascii[*(filename+i)];

    for (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((const char*)extrafield_global)+i);

    for (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = __ebcdic_to_ascii [*(comment+i)];

    if (zi->ci.central_header == NULL) {
        zi->filezip = tfh;
        return ZIP_INTERNALERROR;
    };

    /* write the local header */
    err = ziplocal_putValue(zi->filezip,(uLong)LOCALHEADERMAGIC,4);

    if (err==ZIP_OK)
        /* version needed to extract */
        err = ziplocal_putValue(zi->filezip,(uLong)20,2);
    
    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.flag,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.method,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)zi->ci.dosDate,4);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); /* crc 32, unknown */
    
    if (err==ZIP_OK)
        /* compressed size, unknown */
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); 
    
    if (err==ZIP_OK)
        /* uncompressed size, unknown */
        err = ziplocal_putValue(zi->filezip,(uLong)0,4); 

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)size_filename,2);

    if (err==ZIP_OK)
        err = ziplocal_putValue(zi->filezip,(uLong)size_extrafield_local,2);

    if ((err==ZIP_OK) && (size_filename>0)) {
        
        strcpy (tstr, filename);
        etoa (tstr, size_filename);

        if (fwrite(tstr,(uInt)size_filename,1,zi->filezip)!=1)
                err = ZIP_ERRNO;
    };

    if ((err==ZIP_OK) && (size_extrafield_local>0))
        if (fwrite(extrafield_local,(uInt)size_extrafield_local,1,zi->filezip)
                                                                           !=1)
                err = ZIP_ERRNO;

    zi->ci.stream.avail_in = (uInt)0;
    zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.stream.next_out = zi->ci.buffered_data;
    zi->ci.stream.total_in = 0;
    zi->ci.stream.total_out = 0;

    if ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED))
    {
        zi->ci.stream.zalloc = (alloc_func)0;
        zi->ci.stream.zfree = (free_func)0;
        zi->ci.stream.opaque = (voidpf)0;

        err = deflateInit2(&zi->ci.stream, level,
               Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, 0);

        if (err==Z_OK)
            zi->ci.stream_initialised = 1;
    }

    if (err==Z_OK)
        zi->in_opened_file_inzip = 1;

    zi->filezip = tfh;
    
    return err;
}

extern int ZEXPORT zipWriteInFileInZip (file, buf, len)
    zipFile file;
    const voidp buf;
    unsigned len;
{
    zip_internal* zi;
    int err=ZIP_OK;
    FILE * tfh;

    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 0)
        return ZIP_PARAMERROR;

    tfh = zi->filezip;
    zi->filezip = t1fh;

    zi->ci.stream.next_in = buf;
    zi->ci.stream.avail_in = len;
    zi->ci.crc32 = crc32(zi->ci.crc32,buf,len);

    while ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
    {
        if (zi->ci.stream.avail_out == 0)
        {
            if (fwrite(zi->ci.buffered_data,
                (uInt)zi->ci.pos_in_buffered_data,1,zi->filezip) !=1)
                err = ZIP_ERRNO;
            zi->ci.pos_in_buffered_data = 0;
            zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
            zi->ci.stream.next_out = zi->ci.buffered_data;
        }

        if (zi->ci.method == Z_DEFLATED)
        {
            uLong uTotalOutBefore = zi->ci.stream.total_out;
            err=deflate(&zi->ci.stream,  Z_NO_FLUSH);
            zi->ci.pos_in_buffered_data += 
                (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;

        }
        else
        {
            uInt copy_this,i;
            if (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                copy_this = zi->ci.stream.avail_in;
            else
                copy_this = zi->ci.stream.avail_out;
            for (i=0;i<copy_this;i++)
                *(((char*)zi->ci.stream.next_out)+i) =
                    *(((const char*)zi->ci.stream.next_in)+i);
            {
                zi->ci.stream.avail_in -= copy_this;
                zi->ci.stream.avail_out-= copy_this;
                zi->ci.stream.next_in+= copy_this;
                zi->ci.stream.next_out+= copy_this;
                zi->ci.stream.total_in+= copy_this;
                zi->ci.stream.total_out+= copy_this;
                zi->ci.pos_in_buffered_data += copy_this;
            }
        }
    }

    zi->filezip = tfh;
    return 0;
}

extern int ZEXPORT zipCloseFileInZip (file)
    zipFile file;
{
    zip_internal* zi;
    int err=ZIP_OK;
    FILE * tfh;

    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 0)    
        return ZIP_PARAMERROR;
    
    tfh = zi->filezip;
    zi->filezip = t1fh;

    zi->ci.stream.avail_in = 0;
    
    if (zi->ci.method == Z_DEFLATED)
        while (err==ZIP_OK)
    {
        uLong uTotalOutBefore;
        if (zi->ci.stream.avail_out == 0)
        {
            if (fwrite(zi->ci.buffered_data,
                        (uInt)zi->ci.pos_in_buffered_data,1,zi->filezip) !=1)
                err = ZIP_ERRNO;
            zi->ci.pos_in_buffered_data = 0;
            zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
            zi->ci.stream.next_out = zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.stream.total_out;
        err=deflate(&zi->ci.stream,  Z_FINISH);
        zi->ci.pos_in_buffered_data += 
                (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
    }

    if (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    if ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
        if (fwrite(zi->ci.buffered_data,
                (uInt)zi->ci.pos_in_buffered_data,1,zi->filezip) !=1)
            err = ZIP_ERRNO;

    if ((zi->ci.method == Z_DEFLATED) && (err==ZIP_OK))
    {
        err=deflateEnd(&zi->ci.stream);
        zi->ci.stream_initialised = 0;
    }

    /*crc*/
    ziplocal_putValue_inmemory(zi->ci.central_header+16,
                                (uLong)zi->ci.crc32,4); 
    /*compr size*/
    ziplocal_putValue_inmemory(zi->ci.central_header+20,
                                (uLong)zi->ci.stream.total_out,4); 
    /*uncompr size*/                            
    ziplocal_putValue_inmemory(zi->ci.central_header+24,
                                (uLong)zi->ci.stream.total_in,4); 

    if (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir,zi->ci.central_header,
                                       (uLong)zi->ci.size_centralheader);
    free(zi->ci.central_header);

    g_ftell = (unsigned long)zi->ci.pos_local_header + 14 - g_offset;
    g_crc32 = (unsigned long)zi->ci.crc32;
    g_csize = (unsigned long)zi->ci.stream.total_out;
    g_usize = (unsigned long)zi->ci.stream.total_in;

    zi->number_entry ++;
    zi->in_opened_file_inzip = 0;

    zi->filezip = tfh;

    return err;
}

extern int ZEXPORT zipClose (file, global_comment)
    zipFile file;
    const char* global_comment;
{
    zip_internal* zi;
    int err = 0;
    uLong size_centraldir = 0;
    uLong centraldir_pos_inzip ;
    uInt size_global_comment;
    if (file == NULL)
        return ZIP_PARAMERROR;
    zi = (zip_internal*)file;

    if (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

    if (global_comment==NULL)
        size_global_comment = 0;
    else
        size_global_comment = strlen(global_comment);


    centraldir_pos_inzip = ftell(zi->filezip);
    if (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block ;
        while (ldi!=NULL)
        {
            if ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
                if (fwrite(ldi->data,(uInt)ldi->filled_in_this_block,
                                        1,zi->filezip) !=1 )
                    err = ZIP_ERRNO;

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_datablock(zi->central_dir.first_block);

    if (err==ZIP_OK) /* Magic End */
        err = ziplocal_putValue(zi->filezip,(uLong)ENDHEADERMAGIC,4);

    if (err==ZIP_OK) /* number of this disk */
        err = ziplocal_putValue(zi->filezip,(uLong)0,2);

    if (err==ZIP_OK) 
        /* number of the disk with the start of the central directory */
        err = ziplocal_putValue(zi->filezip,(uLong)0,2);

    if (err==ZIP_OK) 
        /* total number of entries in the central dir on this disk */
        err = ziplocal_putValue(zi->filezip,(uLong)zi->number_entry,2);

    if (err==ZIP_OK) /* total number of entries in the central dir */
        err = ziplocal_putValue(zi->filezip,(uLong)zi->number_entry,2);

    if (err==ZIP_OK) /* size of the central directory */
        err = ziplocal_putValue(zi->filezip,(uLong)size_centraldir,4);

    if (err==ZIP_OK) 
    /* offset of start of central directory with 
       respect to the starting disk number */
        err = ziplocal_putValue(zi->filezip,(uLong)centraldir_pos_inzip ,4);

    if (err==ZIP_OK) /* zipfile comment length */
        err = ziplocal_putValue(zi->filezip,(uLong)size_global_comment,2);

    if ((err==ZIP_OK) && (size_global_comment>0))
        if (fwrite(global_comment,
                (uInt)size_global_comment,1,zi->filezip) !=1 )
                err = ZIP_ERRNO;
    fclose(zi->filezip);
    TRYFREE(zi);

    return err;
}
