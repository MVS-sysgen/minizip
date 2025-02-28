# Makefile for generating minizip JCL compilation stream
# Paths
SCRIPT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SRC := $(SCRIPT_DIR)/src
MINIZIP := $(SRC)/contrib/minizip
BUILD := $(SCRIPT_DIR)/build

# Update accordingly
MVSCE := ./MVSCE
SCRIPT := $(BUILD)/makexmi.py

# Target file
TARGET := compile.jcl
MINIZIP_XMI := MINIZIP.XMI

# Build type flags
ifeq ($(MAKECMDGOALS),release)
    RELEASE_BUILD := 1
endif

# Source files
C_SOURCES := $(wildcard $(SRC)/*.c) $(wildcard $(MINIZIP)/*.c)
H_SOURCES := $(wildcard $(SRC)/*.h) $(wildcard $(MINIZIP)/*.h)

# Build type flags
#RELEASE_BUILD :=

.PHONY: all clean release install

all: $(TARGET)

release: RELEASE_BUILD := 1
release: $(TARGET) $(MINIZIP_XMI)

install:
	@echo "[+] Running install command"
	@$(SCRIPT) -d -m $(MVSCE)

$(MINIZIP_XMI): clean
	@echo "[+] Creating MINIZIP.XMI for release"
	@$(SCRIPT) -d -m $(MVSCE) --release

$(TARGET): clean
	@echo "[+] Putting required files in a single jobstream"
	@echo "[+] Script dir:  $(SCRIPT_DIR)"
	@echo "[+] Source dir:  $(SRC)"
	@echo "[+] Minizip dir: $(MINIZIP)"
	
	# Initial JCL header
	@echo '//MINIZIP  JOB (TSO),' > $(TARGET)
	@echo '//             '\''Install Minizip'\'',' >> $(TARGET)
	@echo '//             CLASS=A,' >> $(TARGET)
	@echo '//             MSGCLASS=A,' >> $(TARGET)
	@echo '//             MSGLEVEL=(1,1),' >> $(TARGET)
	@echo '//             USER=IBMUSER,' >> $(TARGET)
	@echo '//             PASSWORD=SYS1,REGION=0M' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//*  Compiles MINIZIP and MINIUZIP' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//* ---------------------------------------------------------------------------' >> $(TARGET)
	@echo '//* ' >> $(TARGET)
	@echo '//* CLEANUP' >> $(TARGET)
	@echo '//STEP01 EXEC PGM=IDCAMS' >> $(TARGET)
	@echo '//SYSPRINT DD SYSOUT=*' >> $(TARGET)
	@echo '//SYSIN DD *' >> $(TARGET)
	@echo ' DELETE '\''MVP.INCLUDE.TEMP'\'' PURGE' >> $(TARGET)
	@echo ' DELETE '\''MVP.SOURCE.TEMP'\'' PURGE' >> $(TARGET)
	@echo ' DELETE '\''MVP.NCALIB.TEMP'\'' PURGE' >> $(TARGET)
ifdef RELEASE_BUILD
	@echo ' DELETE '\''SOF.MINIZIP'\'' PURGE' >> $(TARGET)
endif
	@echo '' >> $(TARGET)
	@echo ' SET LASTCC=0' >> $(TARGET)
	@echo ' SET MAXCC=0' >> $(TARGET)
	@echo '/*' >> $(TARGET)
	
	# Add C files section
	@echo '//*' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//* Add the C Files' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//STEP1   EXEC PGM=PDSLOAD' >> $(TARGET)
	@echo '//STEPLIB  DD  DSN=SYSC.LINKLIB,DISP=SHR' >> $(TARGET)
	@echo '//SYSPRINT DD  SYSOUT=*' >> $(TARGET)
	@echo '//SYSUT2   DD  DSN=MVP.SOURCE.TEMP,DISP=(NEW,CATLG,DELETE),' >> $(TARGET)
	@echo '//             VOL=SER=PUB000,' >> $(TARGET)
	@echo '//             UNIT=SYSDA,SPACE=(CYL,(2,3,14)),' >> $(TARGET)
	@echo '//             DCB=(RECFM=FB,LRECL=80,BLKSIZE=19040)' >> $(TARGET)
	@echo '//SYSUT1   DD  DATA,DLM=@@' >> $(TARGET)
	
	# Add C source files
	@for file in $(C_SOURCES); do \
		echo "adding $$file"; \
		basename=$$(basename $$file .c); \
		echo "./ ADD NAME=$$(echo $$basename | tr '[:lower:]' '[:upper:]')" >> $(TARGET); \
		cat $$file >> $(TARGET); \
		echo "" >> $(TARGET); \
	done
	
	@echo '@@' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	
	# Add header files section
	@echo '//*' >> $(TARGET)
	@echo '//* ---------------------------------------------------------------------------' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//* Add the header Files' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//STEP1   EXEC PGM=PDSLOAD' >> $(TARGET)
	@echo '//STEPLIB  DD  DSN=SYSC.LINKLIB,DISP=SHR' >> $(TARGET)
	@echo '//SYSPRINT DD  SYSOUT=*' >> $(TARGET)
	@echo '//SYSUT2   DD  DSN=MVP.INCLUDE.TEMP,DISP=(NEW,CATLG,DELETE),' >> $(TARGET)
	@echo '//             VOL=SER=PUB000,' >> $(TARGET)
	@echo '//             UNIT=SYSDA,SPACE=(CYL,(2,3,14)),' >> $(TARGET)
	@echo '//             DCB=(RECFM=FB,LRECL=80,BLKSIZE=19040)' >> $(TARGET)
	@echo '//SYSUT1   DD  DATA,DLM=@@' >> $(TARGET)
	
	# Add header files
	@for file in $(H_SOURCES); do \
		echo "adding $$file"; \
		basename=$$(basename $$file .h); \
		echo "./ ADD NAME=$$(echo $$basename | tr '[:lower:]' '[:upper:]')" >> $(TARGET); \
		cat $$file >> $(TARGET); \
		echo "" >> $(TARGET); \
	done
	
	@echo '@@' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	
	# Add compilation and linking JCL
	@echo '//*' >> $(TARGET)
	@echo '//* ---------------------------------------------------------------------------' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//* Compile Minizip/Miniunzip ' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//*' >> $(TARGET)
	@echo '//NCALIB   EXEC PGM=IEFBR14' >> $(TARGET)
	@echo '//NCALIB   DD   DSN=MVP.NCALIB.TEMP,DISP=(NEW,CATLG,DELETE),' >> $(TARGET)
	@echo '//   DCB=(RECFM=U,LRECL=0,BLKSIZE=6144),' >> $(TARGET)
	@echo '//   SPACE=(6144,(29,29,44)),UNIT=SYSDA' >> $(TARGET)
ifdef RELEASE_BUILD
	@echo '//SOFLIB  DD DSN=SOF.MINIZIP,' >> $(TARGET)
	@echo '//            DISP=(NEW,CATLG,DELETE),' >> $(TARGET)
	@echo '//            UNIT=3390,VOL=SER=PUB001,' >> $(TARGET)
	@echo '//            SPACE=(TRK,(3,5,5)),' >> $(TARGET)
	@echo '//            DCB=(RECFM=U,BLKSIZE=32760)' >> $(TARGET)
endif
	@echo "//* GCC Proc" >> $(TARGET)
	@echo "//MINICMP   PROC MEMBER='',GCCPREF='GCC'," >> $(TARGET)
	@echo "// PDPPREF='PDPCLIB'," >> $(TARGET)
	@echo "// COS1='-Os -S -ansi'," >> $(TARGET)
	@echo "// COS2='-o dd:out -'" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//COMP     EXEC PGM=GCC,PARM='&COS1 &COS2'" >> $(TARGET)
	@echo "//STEPLIB  DD DSN=&GCCPREF..LINKLIB,DISP=SHR" >> $(TARGET)
	@echo "//SYSIN    DD DSN=MVP.SOURCE.TEMP(&MEMBER),DISP=SHR" >> $(TARGET)
	@echo "//INCLUDE  DD DSN=MVP.INCLUDE.TEMP,DISP=SHR" >> $(TARGET)
	@echo "//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR" >> $(TARGET)
	@echo "//SYSINCL  DD DSN=MVP.INCLUDE.TEMP,DISP=SHR" >> $(TARGET)
	@echo "//         DD DSN=&PDPPREF..INCLUDE,DISP=SHR" >> $(TARGET)
	@echo "//OUT      DD DSN=&&TEMP,DISP=(NEW,PASS),UNIT=SYSDA," >> $(TARGET)
	@echo "//            DCB=(LRECL=80,BLKSIZE=6080,RECFM=FB)," >> $(TARGET)
	@echo "//            SPACE=(6080,(500,500))" >> $(TARGET)
	@echo "//SYSPRINT DD SYSOUT=*" >> $(TARGET)
	@echo "//SYSTERM  DD SYSOUT=*" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//ASM      EXEC PGM=IFOX00," >> $(TARGET)
	@echo "//            PARM='DECK,NOLIST'," >> $(TARGET)
	@echo "//            COND=(4,LT,COMP)" >> $(TARGET)
	@echo "//SYSLIB   DD DSN=SYS1.MACLIB,DISP=SHR,DCB=BLKSIZE=32720" >> $(TARGET)
	@echo "//         DD DSN=&PDPPREF..MACLIB,DISP=SHR" >> $(TARGET)
	@echo "//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(20,10))" >> $(TARGET)
	@echo "//SYSUT2   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))" >> $(TARGET)
	@echo "//SYSUT3   DD UNIT=SYSALLDA,SPACE=(CYL,(10,10))" >> $(TARGET)
	@echo "//SYSPRINT DD SYSOUT=*" >> $(TARGET)
	@echo "//SYSLIN   DD DUMMY" >> $(TARGET)
	@echo "//SYSGO    DD DUMMY" >> $(TARGET)
	@echo "//SYSPUNCH DD DSN=&&OBJSET,UNIT=SYSALLDA,SPACE=(80,(240,200))," >> $(TARGET)
	@echo "//            DISP=(NEW,PASS)" >> $(TARGET)
	@echo "//SYSIN    DD DSN=&&TEMP,DISP=(OLD,DELETE)" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//LKED     EXEC PGM=IEWL,PARM='NCAL'," >> $(TARGET)
	@echo "//            COND=((4,LT,COMP),(4,LT,ASM))" >> $(TARGET)
	@echo "//SYSLIN   DD DSN=&&OBJSET,DISP=(OLD,DELETE)" >> $(TARGET)
	@echo "//SYSLMOD  DD DSN=MVP.NCALIB.TEMP(&MEMBER),DISP=SHR" >> $(TARGET)
	@echo "//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(2,1))" >> $(TARGET)
	@echo "//SYSPRINT DD SYSOUT=*" >> $(TARGET)
	@echo "//         PEND" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//* Compile the members" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//MINIUNZ      EXEC MINICMP,MEMBER=MINIUNZ" >> $(TARGET)
	@echo "//UNZIP        EXEC MINICMP,MEMBER=UNZIP" >> $(TARGET)
	@echo "//UNSHRINK     EXEC MINICMP,MEMBER=UNSHRINK" >> $(TARGET)
	@echo "//EXTRA        EXEC MINICMP,MEMBER=EXTRA" >> $(TARGET)
	@echo "//CRC32        EXEC MINICMP,MEMBER=CRC32" >> $(TARGET)
	@echo "//INFLATE      EXEC MINICMP,MEMBER=INFLATE" >> $(TARGET)
	@echo "//INFBLOCK     EXEC MINICMP,MEMBER=INFBLOCK" >> $(TARGET)
	@echo "//INFCODES     EXEC MINICMP,MEMBER=INFCODES" >> $(TARGET)
	@echo "//INFTREES     EXEC MINICMP,MEMBER=INFTREES" >> $(TARGET)
	@echo "//INFUTIL      EXEC MINICMP,MEMBER=INFUTIL" >> $(TARGET)
	@echo "//INFFAST      EXEC MINICMP,MEMBER=INFFAST" >> $(TARGET)
	@echo "//ZUTIL        EXEC MINICMP,MEMBER=ZUTIL" >> $(TARGET)
	@echo "//ADLER32      EXEC MINICMP,MEMBER=ADLER32" >> $(TARGET)
	@echo "//MINIZIP      EXEC MINICMP,MEMBER=MINIZIP" >> $(TARGET)
	@echo "//ZIP          EXEC MINICMP,MEMBER=ZIP" >> $(TARGET)
	@echo "//DEFLATE      EXEC MINICMP,MEMBER=DEFLATE" >> $(TARGET)
	@echo "//TREES        EXEC MINICMP,MEMBER=TREES" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//* ---------------------------------------------------------------------------" >> $(TARGET)
	@echo "//*" >> $(TARGET)
	@echo "//* Link Minizip/Miniunzip " >> $(TARGET)
	@echo "//LINK     PROC PDPPREF='PDPCLIB',EXE=''" >> $(TARGET)
	@echo "//LINKMINI EXEC PGM=IEWL,PARM='MAP,LIST,SIZE=(999424,65536)'" >> $(TARGET)
	@echo "//SYSUT1   DD UNIT=SYSALLDA,SPACE=(CYL,(30,10))" >> $(TARGET)
	@echo "//SYSPRINT DD SYSOUT=*" >> $(TARGET)
	@echo "//SYSLIB   DD DSN=&PDPPREF..NCALIB,DISP=SHR,DCB=BLKSIZE=32760" >> $(TARGET)
	@echo "//         DD DSN=MVP.NCALIB.TEMP,DISP=SHR" >> $(TARGET)
ifdef RELEASE_BUILD
	@echo "//SYSLMOD  DD DSN=SOF.MINIZIP(&EXE),DISP=SHR" >> $(TARGET)
else
	@echo "//SYSLMOD  DD DSN=SYS2.LINKLIB(&EXE),DISP=SHR" >> $(TARGET)
endif
	@echo "//         PEND" >> $(TARGET)
	@echo "//DOLINK1  EXEC LINK,EXE='MINIUNZ'" >> $(TARGET)
	@echo "//LINKMINI.SYSLIN   DD *" >> $(TARGET)
	@echo " INCLUDE SYSLIB(MINIUNZ)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(UNZIP)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(UNSHRINK)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(EXTRA)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(CRC32)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFLATE)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFBLOCK)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFCODES)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFTREES)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFUTIL)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(INFFAST)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(ZUTIL)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(ADLER32)" >> $(TARGET)
	@echo " ENTRY @@MAIN" >> $(TARGET)
	@echo "/*" >> $(TARGET)
	@echo "//DOLINK2  EXEC LINK,EXE='MINIZIP'" >> $(TARGET)
	@echo "//LINKMINI.SYSLIN   DD *" >> $(TARGET)
	@echo " INCLUDE SYSLIB(MINIZIP)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(ZIP)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(DEFLATE)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(CRC32)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(ADLER32)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(TREES)" >> $(TARGET)
	@echo " INCLUDE SYSLIB(ZUTIL)" >> $(TARGET)
	@echo " ENTRY @@MAIN" >> $(TARGET)
	@echo "/*" >> $(TARGET)

clean:
	@rm -f $(TARGET) MINIZIP.XMI minizip.xmit.punch