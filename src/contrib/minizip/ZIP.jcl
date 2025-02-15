//ZIP    PROC OUTZIP=,INFILE=,P=                                      
//*                                                                   
//* JCL procedure for MINIZIP                                         
//*                                                                   
//* to use it pass the provide the location of the zip file           
//* to be creted to OUTZIP                                            
//* INFILE is the DSN to be compressed                                
//* P is for flags or other parms                                     
//*                                                                   
//* e.g. //ZIP EXEC ZIP,P='-o',OUTZIP='IBMUSER.ZIP',INFILE='TEST.TEST'
//*                                                                   
//ZIP      EXEC PGM=MINIZIP,REGION=0M,                                
//  PARM='&P DD:ZIPFILE &INFILE'                                      
//ZIPFILE DD DISP=(NEW,CATLG,DELETE),                                 
//        DSN=&OUTZIP,UNIT=SYSDA,                                     
//        VOL=SER=PUB001,SPACE=(TRK,(15,15),RLSE),                    
//        DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=27920)              
//STDOUT   DD   SYSOUT=*                                              
//SYSPRINT DD   SYSOUT=*                                              
//SYSTERM  DD   SYSOUT=*                                              
//SYSIN    DD   DUMMY                                                 
//SYSUT1   DD   UNIT=SYSDA,SPACE=(CYL,300),                           
//  DCB=(DSORG=PS,RECFM=FB,LRECL=128,BLKSIZE=6144)                    