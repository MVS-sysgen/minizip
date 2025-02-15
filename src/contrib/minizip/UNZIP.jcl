//UNZIP    PROC OUTDSN=,INZIP=,P=                                 
//*
//* JCL procedure for miniunz
//*
//* to use it pass the input zip file (must be a seq dataset)
//* and the output PDS
//*
//UNZIP    EXEC PGM=MINIUNZ,REGION=0M,              
//  PARM='&P &INZIP DD:OUTDSN'  
//OUTDSN    DD  DSN=&OUTDSN,DISP=(,CATLG),
//             UNIT=SYSDA,VOL=SER=PUB001,
//             SPACE=(CYL,(10,5,50)),                   
//             DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=27920)                   
//STDOUT   DD   SYSOUT=*                           
//SYSPRINT DD   SYSOUT=*                           
//SYSTERM  DD   SYSOUT=*                           
//SYSIN    DD   DUMMY                              
//SYSUT1   DD   UNIT=SYSDA,SPACE=(CYL,300),        
//  DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)