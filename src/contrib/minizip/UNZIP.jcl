//UNZIP    PROC P=                                 
//*
//* JCL procedure for miniunz
//*
//* to use it pass the input zip file (must be a seq dataset)
//* and the output PDS
//* 
//* e.g. //UNZIP EXEC UNZIP,P='IBMUSER.TEST.ZIP IBMUSER.OUT.PDS'
//*
//UNZIP    EXEC PGM=MINIUNZ,REGION=0M,              
//  PARM='&P'                                    
//STDOUT   DD   SYSOUT=*                           
//SYSPRINT DD   SYSOUT=*                           
//SYSTERM  DD   SYSOUT=*                           
//SYSIN    DD   DUMMY                              
//SYSUT1   DD   UNIT=SYSDA,SPACE=(CYL,300),        
//  DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)