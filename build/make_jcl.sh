#!/bin/bash

SCRIPT_DIR=$(dirname "$0")
SRC=$SCRIPT_DIR"/../src"
MINIZIP=$SRC"/contrib/minizip"
echo "[+] Putting required files in a single jobstream"
echo "[+] Script dir:  " $SCRIPT_DIR
echo "[+] Source dir:  " $SRC
echo "[+] Minizip dir: " $MINIZIP


cat > compile.jcl << END
//MINIZIP  JOB (TSO),
//             'Install Minizip',
//             CLASS=A,
//             MSGCLASS=A,
//             MSGLEVEL=(1,1),
//             USER=IBMUSER,
//             PASSWORD=SYS1,REGION=0M
//*
//*  Compiles MINIZIP and MINIUZIP
//*
//* ---------------------------------------------------------------------------
//* 
//* CLEANUP
//STEP01 EXEC PGM=IDCAMS
//SYSPRINT DD SYSOUT=*
//SYSIN DD *
 DELETE 'MVP.INCLUDE.TEMP' PURGE
 DELETE 'MVP.SOURCE.TEMP' PURGE
 DELETE 'MVP.NCALIB.TEMP' PURGE
 SET LASTCC=0
 SET MAXCC=0
/*
//*
//*
//* Add the C Files
//*
//STEP1   EXEC PGM=PDSLOAD
//STEPLIB  DD  DSN=SYSC.LINKLIB,DISP=SHR
//SYSPRINT DD  SYSOUT=*
//SYSUT2   DD  DSN=MVP.SOURCE.TEMP,DISP=(NEW,CATLG,DELETE),
//             VOL=SER=PUB000,
//             UNIT=SYSDA,SPACE=(CYL,(2,3,14)),
//             DCB=(RECFM=FB,LRECL=80,BLKSIZE=19040)
//SYSUT1   DD  DATA,DLM=@@
END

for i in $SRC/*.c $MINIZIP/*.c; do
    echo "[+] adding $i"
    i=$(readlink -f "$i") # Handle spaces and symlinks
    m="${i%.*}"
    member="${m##*/}"

    echo "./ ADD NAME=${member^^}" >> compile.jcl

    if [ -r "$i" ]; then
        cat "$i" >> compile.jcl
        # Add the newline *after* the cat, but only to compile.jcl
        echo "" >> compile.jcl  # Append an empty line (newline) to compile.jcl
    else
        echo "Error: File '$i' is not readable." >&2
    fi
done
echo '@@' >> compile.jcl
echo "//*" >> compile.jcl
cat >> compile.jcl << END
//*
//* ---------------------------------------------------------------------------
//*
//* Add the header Files
//*
//STEP1   EXEC PGM=PDSLOAD
//STEPLIB  DD  DSN=SYSC.LINKLIB,DISP=SHR
//SYSPRINT DD  SYSOUT=*
//SYSUT2   DD  DSN=MVP.INCLUDE.TEMP,DISP=(NEW,CATLG,DELETE),
//             VOL=SER=PUB000,
//             UNIT=SYSDA,SPACE=(CYL,(2,3,14)),
//             DCB=(RECFM=FB,LRECL=80,BLKSIZE=19040)
//SYSUT1   DD  DATA,DLM=@@
END

for i in $SRC/*.h $MINIZIP/*.h; do
    echo "[+] adding $i"
    i=$(readlink -f "$i") # Handle spaces and symlinks
    m="${i%.*}"
    member="${m##*/}"

    echo "./ ADD NAME=${member^^}" >> compile.jcl

    if [ -r "$i" ]; then
        cat "$i" >> compile.jcl
        # Add the newline *after* the cat, but only to compile.jcl
        echo "" >> compile.jcl  # Append an empty line (newline) to compile.jcl
    else
        echo "Error: File '$i' is not readable." >&2
    fi
done
echo '@@' >> compile.jcl
echo "//*" >> compile.jcl

echo "[+] Adding GCC and linker JCL"
cat >> compile.jcl << 'END'
//*
//* ---------------------------------------------------------------------------
//*
//* Compile Minizip/Miniunzip 
//*
//*
//NCALIB   EXEC PGM=IEFBR14
//NCALIB   DD   DSN=MVP.NCALIB.TEMP,DISP=(NEW,CATLG,DELETE),
//   DCB=(RECFM=U,LRECL=0,BLKSIZE=6144),
//   SPACE=(6144,(29,29,44)),UNIT=SYSDA
//MINICMP   PROC MEMBER='',GCCPREF='GCC',
// PDPPREF='PDPCLIB',
// COS1='-Os -S -ansi',
// COS2='-o dd:out -'
//*
//COMP     EXEC PGM=GCC,PARM='&COS1 &COS2'
//STEPLIB  DD DSN=&GCCPREF..LINKLIB,DISP=SHR
//SYSIN    DD DSN=MVP.SOURCE.TEMP(&MEMBER),DISP=SHR
//INCLUDE  DD DSN=MVP.INCLUDE.TEMP,DISP=SHR
//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//SYSINCL  DD DSN=MVP.INCLUDE.TEMP,DISP=SHR
//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR
//OUT      DD DSN=&&TEMP,DISP=(NEW,PASS),UNIT=SYSDA,
//            DCB=(LRECL=80,BLKSIZE=6080,RECFM=FB),
//            SPACE=(6080,(500,500))
//SYSPRINT DD SYSOUT=*
//SYSTERM  DD SYSOUT=*
//*
//ASM      EXEC PGM=IFOX00,
//            PARM='DECK,NOLIST',
//            COND=(4,LT,COMP)
//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720
//         DD DSN=&PDPPREF..MACLIB,DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))
//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))
//SYSPRINT DD SYSOUT=*
//SYSLIN   DD DUMMY
//SYSGO    DD DUMMY
//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(240,200)),
//            DISP=(NEW,PASS)
//SYSIN    DD DSN=&&TEMP,DISP=(OLD,DELETE)
//*
//LKED     EXEC PGM=IEWL,PARM='NCAL',
//            COND=((4,LT,COMP),(4,LT,ASM))
//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)
//SYSLMOD  DD DSN=MVP.NCALIB.TEMP(&MEMBER),DISP=SHR
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))
//SYSPRINT DD SYSOUT=*
//         PEND
//*
//* Compile the members
//*
//MINIUNZ      EXEC MINICMP,MEMBER=MINIUNZ
//UNZIP        EXEC MINICMP,MEMBER=UNZIP
//UNSHRINK     EXEC MINICMP,MEMBER=UNSHRINK
//EXTRA        EXEC MINICMP,MEMBER=EXTRA
//CRC32        EXEC MINICMP,MEMBER=CRC32
//INFLATE      EXEC MINICMP,MEMBER=INFLATE
//INFBLOCK     EXEC MINICMP,MEMBER=INFBLOCK
//INFCODES     EXEC MINICMP,MEMBER=INFCODES
//INFTREES     EXEC MINICMP,MEMBER=INFTREES
//INFUTIL      EXEC MINICMP,MEMBER=INFUTIL
//INFFAST      EXEC MINICMP,MEMBER=INFFAST
//ZUTIL        EXEC MINICMP,MEMBER=ZUTIL
//ADLER32      EXEC MINICMP,MEMBER=ADLER32
//MINIZIP      EXEC MINICMP,MEMBER=MINIZIP
//ZIP          EXEC MINICMP,MEMBER=ZIP
//DEFLATE      EXEC MINICMP,MEMBER=DEFLATE
//TREES        EXEC MINICMP,MEMBER=TREES
//*
//* ---------------------------------------------------------------------------
//*
//* Link Minizip/Miniunzip 
//LINK     PROC PDPPREF='PDPCLIB',EXE=''
//LKED     EXEC PGM=IEWL,PARM='MAP,LIST,SIZE=(999424,65536)'
//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(30,10))
//SYSPRINT DD SYSOUT=*
//SYSLIB   DD DSN=&PDPPREF..NCALIB,DISP=SHR,DCB=BLKSIZE=32760
//         DD DSN=MVP.NCALIB.TEMP,DISP=SHR
//SYSLMOD  DD DSN=SYS2.LINKLIB(&EXE),DISP=SHR
//         PEND
//DOLINK1  EXEC LINK,EXE='MINIUNZ'
//LKED.SYSLIN   DD *
 INCLUDE SYSLIB(MINIUNZ)
 INCLUDE SYSLIB(UNZIP)
 INCLUDE SYSLIB(UNSHRINK)
 INCLUDE SYSLIB(EXTRA)
 INCLUDE SYSLIB(CRC32)
 INCLUDE SYSLIB(INFLATE)
 INCLUDE SYSLIB(INFBLOCK)
 INCLUDE SYSLIB(INFCODES)
 INCLUDE SYSLIB(INFTREES)
 INCLUDE SYSLIB(INFUTIL)
 INCLUDE SYSLIB(INFFAST)
 INCLUDE SYSLIB(ZUTIL)
 INCLUDE SYSLIB(ADLER32)
 ENTRY @@MAIN
/*
//DOLINK2  EXEC LINK,EXE='MINIZIP'
//LKED.SYSLIN   DD *
 INCLUDE SYSLIB(MINIZIP)
 INCLUDE SYSLIB(ZIP)
 INCLUDE SYSLIB(DEFLATE)
 INCLUDE SYSLIB(CRC32)
 INCLUDE SYSLIB(ADLER32)
 INCLUDE SYSLIB(TREES)
 INCLUDE SYSLIB(ZUTIL)
 ENTRY @@MAIN
/*
END

echo "[!] Compiled and linked binaries will be in SYS2.LINKLIB"
echo "[+] Done, check compile.jcl"