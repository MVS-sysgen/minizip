#!/usr/bin/env python3
# This file automates the install for use with Docker

from pathlib import Path
import sys
from automvs import automation
import logging
import argparse
import os

cwd = os.getcwd()

xmitout = '''//MINIXMIT JOB (TSO),
//             'HEADER FOR MVP',
//             CLASS=A,
//             MSGCLASS=A,
//             MSGLEVEL=(1,1),
//             USER=IBMUSER,PASSWORD=SYS1
//PDSLOAD  EXEC PGM=PDSLOAD
//STEPLIB  DD  DSN=SYSC.LINKLIB,DISP=SHR
//SYSPRINT DD  SYSOUT=*
//SYSUT2  DD DSN=XMIT.TEMP.DATASET,DISP=(NEW,PASS,DELETE),
//             UNIT=SYSDA,VOL=SER=PUB001,SPACE=(TRK,(50,50,10)),
//             DCB=(RECFM=FB,LRECL=80,BLKSIZE=800)
//SYSUT1   DD  DATA,DLM=@@
./ ADD NAME=UNZIP
{unzip}
./ ADD NAME=ZIP
{zip}
./ ADD NAME=README
## Contents

This XMI file contains:
- UNZIP - JCL Proc to UNZIP uncompressed a file
- ZIP - JCL Proc to ZIP compress a file
- README - This file
- MINIXMI - an XMIT of the MINIZIP and MINIUNZ programs

---------------------

{readme}
@@
//* Package the loadlib
//XMIT1 EXEC PGM=XMIT370
//STEPLIB  DD DSN=SYSC.LINKLIB,DISP=SHR
//XMITLOG  DD SYSOUT=*
//SYSPRINT DD SYSOUT=*
//SYSIN    DD DUMMY
//SYSUT1   DD DSN=SOF.MINIZIP,DISP=SHR
//SYSUT2   DD DSN=&&SYSUT2,UNIT=SYSDA,
//         SPACE=(TRK,(255,255)),
//         DISP=(NEW,DELETE,DELETE)
//XMITOUT  DD DSN=XMIT.TEMP.DATASET(MINIXMI),DISP=(OLD,PASS)
//* send it to punch out
//XMIT2 EXEC PGM=XMIT370
//STEPLIB  DD DSN=SYSC.LINKLIB,DISP=SHR
//XMITLOG  DD SYSOUT=*
//SYSPRINT DD SYSOUT=*
//SYSIN    DD DUMMY
//SYSUT1   DD DSN=XMIT.TEMP.DATASET,DISP=(OLD,DELETE)
//SYSUT2   DD DSN=&&SYSUT2,UNIT=SYSDA,
//         SPACE=(TRK,(255,255)),
//         DISP=(NEW,DELETE,DELETE)
//XMITOUT  DD SYSOUT=B
'''


desc = 'Automated XMI Builder'
arg_parser = argparse.ArgumentParser(description=desc)
arg_parser.add_argument('-d', '--debug', help="Print debugging statements", action="store_const", dest="loglevel", const=logging.DEBUG, default=logging.WARNING)
arg_parser.add_argument('-m', '--mvsce', help="MVS/CE folder location", default="MVSCE")
arg_parser.add_argument('-j', '--jcl', help="MVS/CE folder location", default=f"{cwd}/compile.jcl")
arg_parser.add_argument('-r', '--release', 
                       help="Run in release mode (default: test mode)", 
                       action="store_true", 
                       default=False)
args = arg_parser.parse_args()

builder = automation(system_path=args.mvsce,loglevel=args.loglevel)

max_cc = {  'LKED.MINIUNZ' : '0004',
            "LKED.UNZIP" : "0004",
            "LKED.UNSHRINK" : "0004",
            "LKED.EXTRA" : "0004",
            "LKED.INFLATE" : "0004",
            "LKED.INFBLOCK" : "0004",
            "LKED.INFCODES" : "0004",
            "LKED.INFFAST" : "0004",
            "LKED.MINIZIP" : "0004",
            "LKED.ZIP" : "0004",
            "LKED.ZUTIL" : "0004",
            "LKED.DEFLATE" : "0004"
 }

try:
    builder.ipl(clpa=False)
    with open(args.jcl, "r") as infile:
        builder.submit(infile.read())
    builder.wait_for_job("MINIZIP")
    builder.check_maxcc("MINIZIP",steps_cc=max_cc)
    if args.release:
        builder.send_oper("$s punch1")
        builder.change_punchcard_output("{}/minizip.xmit.punch".format(cwd))
        
        builder.wait_for_string("$HASP000 OK")

        with open(f"{cwd}/src/contrib/minizip/UNZIP.jcl", 'r') as infile:
            unzip = infile.read()
        with open(f"{cwd}/src/contrib/minizip/ZIP.jcl", 'r') as infile:
            zip = infile.read()
        with open(f"{cwd}/README.md", 'r') as infile:
            readme = infile.read()

        builder.submit(xmitout.format(unzip=unzip,zip=zip,readme=readme))
        builder.wait_for_string("$HASP190 MINIXMIT SETUP -- PUNCH1   -- F = STD1")
        builder.send_oper("$s punch1")
        builder.wait_for_string("HASP250 MINIXMIT IS PURGED")
        with open("{}/minizip.xmit.punch".format(cwd), 'rb') as punchfile:
            punchfile.seek(160)
            no_headers = punchfile.read()
            no_footers = no_headers[:-80]

        print(f"Writting {cwd}/MINIZIP.XMI")
        with open(f"{cwd}/MINIZIP.XMI", 'wb') as xmi_out:
            xmi_out.write(no_footers)
finally:
    builder.quit_hercules()