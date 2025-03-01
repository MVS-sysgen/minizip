# MVS MINIZIP

This archive contains alterations to minizip/miniunz
files to allow them to be compiled on the MVS platform
with a suitable MVS compiler such as GCCMVS.

Originally from https://sourceforge.net/projects/gccmvs/files/minizip/
this repo took the diff file and applied it to zlib114, then developed
the build script in `./build`. 

# Using zip and unzip:


## Minizip

Minizip can take multiple arguments:

```
MiniZip 0.15 MVS 4.0
Demo of zLib + Zip package written by Gilles Vollant
more info at http://www.winimage.com/zLibDll/minizip.html
Modified for MVS, see http://gccmvs.sourceforge.net
 
Usage : minizip -abco zipfile files_to_add
-a opens files_to_add in text-translated mode and converts EBCDIC to ASCII.
-b zips files without length indicators (use with V,VB or U datasets only.)
-c chooses the alternate code-page 037 instead of the default 1047.
-o specifies that all files_to_add are Partition Organised datasets and
that all members/alias's in each dataset should be zipped.
-l to lowercase names
-x <extension> to add an extension to all filenames
SYSUT1 and zipfile need to be allocated as F/FB with any LRECL and BLKSIZE.
```

To use minizip you need to use JCL. Minizip expects the following:

  - DD for `STDOUT`, `SYSPRINT`, `SYSTERM`, and `SYSIN`
  - DD for `SYSUT1`, which is the work DD
  - A `PARM=` where parm follows the usage described above
    - Note: You must use `DD:` for the output zip file, this 
      will use the DD name in the JCL, in testing giving the
      compressed dataset name as an argument would fail
    - Put one or more datasets to compress
    - You can also give specific members: `SYS1.PROCLIB(ASMCL)`
  - :warning: You **must** Specify `REGION=0M` on the jobcard

Examples:

```jcl
//ZIP JOB (JOB),'ZIP',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//ZIP      EXEC PGM=MINIZIP,REGION=0M,                                
//  PARM='-o DD:ZIPFILE SYS2.PROCLIB SYS1.PROCLIB'                                      
//ZIPFILE DD DISP=(NEW,CATLG,DELETE),                                 
//           DSN=IBMUSER.PROC.ZIP,UNIT=SYSDA,                                     
//           VOL=SER=PUB001,SPACE=(TRK,(15,15),RLSE),                    
//           DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=27920)              
//STDOUT   DD   SYSOUT=*                                              
//SYSPRINT DD   SYSOUT=*                                              
//SYSTERM  DD   SYSOUT=*                                              
//SYSIN    DD   DUMMY                                                 
//SYSUT1   DD   UNIT=SYSDA,SPACE=(CYL,300),                           
//              DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)  
```

```jcl
//ZIP JOB (JOB),'ZIP2',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//ZIP      EXEC PGM=MINIZIP,REGION=0M,                                
//  PARM='DD:ZIPFILE SYS2.PROCLIB(UNZIP) SYS1.PROCLIB(ASMFC)'                                      
//ZIPFILE DD DISP=(NEW,CATLG,DELETE),                                 
//           DSN=IBMUSER.PROC2.ZIP,UNIT=SYSDA,                                     
//           VOL=SER=PUB001,SPACE=(TRK,(15,15),RLSE),                    
//           DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=27920)              
//STDOUT   DD   SYSOUT=*                                              
//SYSPRINT DD   SYSOUT=*                                              
//SYSTERM  DD   SYSOUT=*                                              
//SYSIN    DD   DUMMY                                                 
//SYSUT1   DD   UNIT=SYSDA,SPACE=(CYL,300),                           
//              DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)  
```

## Miniunz

Miniunzip can take multiple arguments:

```
MiniUnz 0.15 MVS 4.0
Demo of zLib + Unz package written by Gilles Vollant
more info at http://www.winimage.com/zLibDll/minizip.html
Modified for MVS - see http://gccmvs.sourceforge.net
 
Usage : miniunz -aclv zipfile dest_file file_to_extract
-a opens files in text-translated mode and converts ASCII to EBCDIC.
-c chooses the alternate code-page 037 instead of the default 1047.
-l or -v only lists statistics and files in the zip archive.
If no file_to_extract is specified, all files are extracted and
the destination file will have (member) automatically appended.
```

Much like MINIZIP you need to use JCL. Miniunz expects the following:

  - DD for `STDOUT`, `SYSPRINT`, `SYSTERM`, and `SYSIN`
  - DD for `SYSUT1`, which is the work DD
  - A `PARM=` where parm follows the usage described above
    - You can use either the dataset names or `DD:` statements
    - The input zip dataset must be a sequential dataset (PS)
    - The output dataset must be a PDS
    - Put one or more datasets to compress
  - :warning: You **must** Specify `REGION=0M` on the jobcard

Examples:

```jcl
//UNZIP JOB (JOB),'UNZIP',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//UNZIP EXEC PGM=MINIUNZ,          
//  PARM='IBMUSER.PROC.ZIP DD:UNZIP'
//UNZIP    DD  DSN=SYS2.PROCLIB,DISP=(,CATLG,),
//             UNIT=SYSDA,VOL=SER=PUB001,
//             SPACE=(CYL,(10,5,50)),                   
//             DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=27920)                               
//STDOUT   DD SYSOUT=*                           
//SYSPRINT DD SYSOUT=*                           
//SYSTERM  DD SYSOUT=*                           
//SYSIN    DD DUMMY                              
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,300),        
// DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)
```


```jcl
//UNZIP JOB (JOB),'UNZIP',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//UNZIP EXEC PGM=MINIUNZ,            
//  PARM='-l IBMUSER.PROC.ZIP'                            
//STDOUT   DD SYSOUT=*                           
//SYSPRINT DD SYSOUT=*                           
//SYSTERM  DD SYSOUT=*                           
//SYSIN    DD DUMMY                              
//SYSUT1   DD UNIT=SYSDA,SPACE=(CYL,300),        
// DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)
```

## Procs

Included in the `src/contrib/minzip` folder are two procs. Put them
in `SYS2.PROCLIB`:

  - `ZIP.jcl` compresses dataset(s)
  - Parms:
    - `OUTZIP` - New zip file to be created
    - `INFILE` - the dataset(s)/members to compress
    - `P` - minizip arguments

  - `UNZIP.jcl` decompresses dataset(s)
  - Parms:
    - `INZIP` - File to be unzipped
    - `OUTDSN` - New PDS to decompress to
    - `P` - miniunz arguments

### Using

**ZIP**

```jcl
//ZIP JOB (JOB),'ZIP',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//ZIP EXEC ZIP,OUTZIP='IBMUSER.PROC.ZIP',
//             INFILE='SYS2.PROCLIB SYS1.PROCLIB',P='-o'  
```

**UNZIP**

```jcl
//UNZIP JOB (JOB),'ZIP',CLASS=A,MSGCLASS=H,NOTIFY=IBMUSER,REGION=0M
//UNZIP EXEC UNZIP,OUTDSN='IBMUSER.PROCTEST',                      
//             INZIP='IBMUSER.PROC.ZIP'                                
```


Usage Notes:
------------

minizip has additional options "-0" to "-9" which select how aggressively the
algorithms check for better compression.  "-0" stores the files with no
compression.

Filenames are assumed to be the DSN unless the dd:xxx format is
used instead.

If the DD name or DSN refers to a Partitioned Organised dataset, then member
names can be added as usual eg. "DD:MYDDNAME(MYMEMBER)"  When zipping with the
"-o" PO dataset option, do not specify a member name as a file_to_add.  When
unzipping all members of a zipfile by not specifying a file_to_extract, do
not specify a member as the dest_file.

When zipping files on the MVS platform, a comment is added with each zipped
file to indicate the dataset format from where the data came.  This includes
an initial character which describes any conversion 'A' is ASCII, 'E' is
for the default of no conversion or EBCDIC data, and 'B' stands for the
special zip mode where only the raw dataset data is stored.

When listing the contents of zip files with -v in miniunz, the comment, if it
exists, is displayed after the filename within braces "{" & "}".

When using text-translated mode, MSDOS style CRLF text files are converted to
just LF when unzipping files.  This conversion is not performed in reverse when
zipping text files on the MVS platform - the resulting text files must be later
converted (eg. by using WinZip and a program like WordPad) if the Windows 
platform is the target.  Note *nix platforms expect only LF characters in 
text files.

There is no provision in minizip to UPDATE an existing zip file.


Building minizip and miniunz from the source code:
--------------------------------------------------

The source for MVS minizip/miniunzip can be found in `src/contrib/minizip`. 

The script in the `build/` folder can be run to generate a file
called `compile.jcl`, submit this job using something like 
`cat compile.jcl | ncat localhost 3505`. This will compile, 
assemble, and link MINIZIP and MINIUNZ to `SYS2.LINKLIB`.

:warning: This JCL is developed for MVS/CE, you will need to 
modify it for TK4- or TK5.


Version:
--------

For version information see minizip.c and miniunz.c or run
the program with or without parameters.


Credit:
-------

Thanks go to Jason Winter for the original archive made for the JCC 
compiler.



See:

Obtain the original zlib 1.1.4 source code from one of
these sites:
http://zlib.net/fossils
ftp://ftp.simplesystems.org/pub/png/src/history/zlib114.zip
http://www.winimage.com/zLibDll/minizip.html
http://www.zlib.net
http://www.gzip.org/zlib


 *** DO NOT REPORT BUGS IN THIS VERSION OF MINI ZIP/UNZIP TO THESE SITES. ***

