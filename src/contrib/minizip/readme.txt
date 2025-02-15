This archive contains alterations to minizip/miniunz
files to allow them to be compiled on the MVS platform
with a suitable MVS compiler such as GCCMVS.

See:

Obtain the original zlib 1.1.4 source code from one of
these sites:
http://zlib.net/fossils
ftp://ftp.simplesystems.org/pub/png/src/history/zlib114.zip
http://www.winimage.com/zLibDll/minizip.html
http://www.zlib.net
http://www.gzip.org/zlib


 *** DO NOT REPORT BUGS IN THIS VERSION OF MINI ZIP/UNZIP TO THESE SITES. ***


This archive is distributed as a zipped patch to the original
zlib. Thus you need to do the following commands (or similar):

md zlib
unzip minizip-0_15-xxx (whatever this distribution is currently called)
cd zlib
unzip ..\zlib114
patch -p 1 <..\minizip-0_15-xxx.txt 
   (or whatever the patch file is currently called)

cd contrib\minizip


Building minizip and miniunz from the source code:
--------------------------------------------------

The source files must be compiled with an MVS C compiler,
and GCCMVS compiler JCL is provided as an example. Getting
the source files onto MVS is beyond the scope of this
document, and is site-specific. The example below is how
it is done in one environment.


The general procedure is to:

1. run zipmvs.bat to get the files required to be sent to the host.

2. submit mini1.jcl to create aliases (if you're not a systems
programmer, you will probably bypass this step).

3. submit mini2.jcl to allocate the target datasets.

4. Transfer the various zip files to the mainframe datasets
via your preferred mechanism. The example mini3.jcl uses
emulated tapes. Your installation may use ftp instead.
The example mini4.jcl extracts the zip files. It assumes that
the "mvsunzip" program is available. If you already have a
compiler available, you can compile mvsunzip.c (part of
PDPCLIB) if you wish to use this.

5. submit mini5.jcl to compile the source.

6. submit mini6.jcl to link the executable.

7. submit mini7.jcl to test the executables and clean up.

There is an example batch file, "allmvs", which may be
useful to do these steps. Also, fixjcl.bat from gccmvs may 
be useful to get the job cards in the JCL correct.


Using zip and unzip:
--------------------

The usage message is displayed when either program is executed as a batch EXEC
or as a TSO command processor (in TSO "alloc fi(stdout) da(*)" is required.)
systerm is required too.

Usage : minizip -abco zipfile files_to_add
-a opens files_to_add in text-translated mode and converts EBCDIC to ASCII.
-b zips files without length indicators (use with V,VB or U datasets only.)
(don't use this option it isn't supported)
-c chooses the alternate code-page 037 instead of the default 1047.
-o specifies that all files_to_add are Partition Organised datasets and
that all members/alias's in each dataset should be zipped.
SYSUT1 and zipfile need to be allocated as F/FB with any LRECL and BLKSIZE.
-l lowercases member names in a PDS
-x <extension> adds an extension to all members of the PDS.

Usage : miniunz -aclv zipfile dest_file file_to_extract
-a opens files in text-translated mode and converts ASCII to EBCDIC.
-c chooses the alternate code-page 037 instead of the default 1047.
-l or -v only lists statistics and files in the zip archive.
If no file_to_extract is specified, all files are extracted and
the destination file will have (member) automatically appended.


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


Version:
--------

For version information see minizip.c and miniunz.c or run
the program with or without parameters.


Credit:
-------

Thanks go to Jason Winter for the original archive made for the JCC 
compiler.


Support:
--------

For MINIZIP for MVS support, try here:
http://tech.groups.yahoo.com/group/hercules-os380/
