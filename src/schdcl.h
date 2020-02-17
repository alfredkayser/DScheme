/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHDCL.H
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/17
**
** DESCRIPTION: Prototype header for the Scheme system
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision: $
** CHANGER:     $Author: $
** WORKFILE:    $Workfile: $
** LOGFILE:     $Logfile: $
** LOGINFO:     $Log: $
**********************************************************************/
#ifndef STATIC
#define STATIC
#endif

/***************** Prototype(s) for SCHEME.C **************************/
int    CDECL   DSinit        __((GLOBAL **, ...));
int    PASCAL  DScheme       __((GLOBAL *));
int    PASCAL  DSextdef      __((GLOBAL *, EXTDEF *));
int    PASCAL  DSmultidef    __((GLOBAL *, EXTDEF *));
int    PASCAL  DSeval        __((GLOBAL *, CELP, CELP *));
int    PASCAL  DSread        __((GLOBAL *, PORT *, CELP *));
int    PASCAL  DSinitport    __((GLOBAL *, PORT **, int, UNTYPE *, INP_FUN, OUT_FUN, CTL_FUN));
int    PASCAL  DS_fopen      __((GLOBAL *, PORT **, CONST char *, int));
int    PASCAL  DSclose       __((GLOBAL **));

int    CDECL   DsWaiter      __((void));
void   PASCAL  DsFuncDef     __((EXTDEF *));

#ifdef MSCX
char * strupr __((char *p));
int stricmp __((char *s1, char *s2));
#endif

/***************** Prototype(s) for SCHPRC.C **************************/

CELP   PASCAL  DsCompare     __((CELP a1, CELP a2));
CELP   PASCAL  DsEqv         __((CELP a1, CELP a2));
CELP   PASCAL  DsEqual       __((CELP a1, CELP a2));
CELP   PASCAL  DsCmpChar     __((CELP a1, CELP a2, int t));
CELP   PASCAL  DsCmpTime     __((CELP a1, CELP a2, int t));
CELP   PASCAL  DsAssQ        __((CELP key, CELP list));
CELP   PASCAL  DsAssV        __((CELP key, CELP list));
CELP   PASCAL  DsAssoc       __((CELP key, CELP list));
CELP   PASCAL  DsMemQ        __((CELP key, CELP list));
CELP   PASCAL  DsMemV        __((CELP key, CELP list));
CELP   PASCAL  DsMember      __((CELP key, CELP list));
CELP   PASCAL  Ds_math1      __((int opcode, CELP arg));
CELP   CDECL   Ds_load       __((CELP name));
LONG   PASCAL  DsLength      __((CELP l));
                                       
/***************** Prototype(s) for SCHCEL.C **************************/
CELP   PASCAL  DsGetCell     __((int typ));
CELP   PASCAL  DsCons        __((CELP a, CELP b));
CELP   PASCAL  DsCons1       __((CELP a));
CELP   PASCAL  DsFltCell     __((REAL v));
CELP   PASCAL  DsIntCell     __((LONG l));
CELP   PASCAL  DsPrtCell     __((PORT *prt));
CELP   PASCAL  DsChrCell     __((int l));
CELP   PASCAL  DsSymbol      __((CONST char *symbol));
CELP   PASCAL  DsInsExt      __((CONST char *name));
CELP   PASCAL  DsStrTime     __((CONST char *str));
void   PASCAL  DsMark        __((CELP p));
void   PASCAL  DsFreeCell    __((CELP temp));
char * PASCAL  DsTimeStr     __((CELP elem));
CELP   PASCAL  DsInsKey      __((CELP key));
CELP   PASCAL  DsTimeReal    __((CELP p));
CELP   PASCAL  DsRealTime    __((CELP p));
CELP   CDECL   DsCar         __((CELP l));
CELP   CDECL   DsCdr         __((CELP l));
CELP   CDECL   DsCaar        __((CELP l));
CELP   CDECL   DsCadr        __((CELP l));
CELP   CDECL   DsCdar        __((CELP l));
CELP   CDECL   DsCddr        __((CELP l));
CELP   CDECL   DsCaaar       __((CELP l));
CELP   CDECL   DsCaadr       __((CELP l));
CELP   CDECL   DsCadar       __((CELP l));
CELP   CDECL   DsCaddr       __((CELP l));
CELP   CDECL   DsCdaar       __((CELP l));
CELP   CDECL   DsCdadr       __((CELP l));
CELP   CDECL   DsCddar       __((CELP l));
CELP   CDECL   DsCdddr       __((CELP l));
void   PASCAL  DsIniCells    __((DWORD numcels));
void   PASCAL  DsAllocCells  __((int blocksize));
void   PASCAL  DsRelease     __((CELP p));
void   PASCAL  DsProtect     __((CELP p));
DWORD  PASCAL  DsGarbageCollect  __((CELP cels));
void   PASCAL  DsCollect     __((void));
void FAR * PASCAL  DsFarMalloc   __((int size));

/***************** Prototype(s) for SCHTIM.C **************************/
      
WORD   PASCAL  DsGetDays     __((int day, int month, int year));
void   PASCAL  DsSetDays     __((int julian, int *day, int *month, int *year));
int    PASCAL  DsDayOfWeek   __((int julian));
CELP   PASCAL  DsMakeTime    __((time_t t));
      
/***************** Prototype(s) for SCHRDR.C **************************/

void   PASCAL  DsIniParser   __((void));
CELP   PASCAL  DsRead        __((PORT *));
CELP   PASCAL  DsStrNumber   __((char *s, int base));
CELP   PASCAL  DsStrReal     __((char *s));
char * PASCAL  ltostr        __((long num, int base));

/****************** Prototype(s) for SCHIO.C **************************/

void   PASCAL  DsIniIO       __((void));
void   PASCAL  DsIniPorts    __((int,int,int,int));
int    PASCAL  DsInput       __((PORT *));
void   PASCAL  DsUnput       __((PORT *, int));
int    PASCAL  DsPeek        __((PORT *));
void   PASCAL  DsOuts        __((PORT *, CONST char *));
void   CDECL   DsOutf        __((PORT *, CONST char *, ...));
void   PASCAL  DsOutc        __((PORT *, CELP, int));
PORT * PASCAL  DsInitPort    __((int, UNTYPE*, INP_FUN, OUT_FUN, CTL_FUN));
PORT * PASCAL  DsFOpen       __((CONST char *, int));
void   PASCAL  DsClosePort   __((PORT *));
void   PASCAL  DsFlushPort   __((PORT *));
void   PASCAL  DsStdPorts    __((void));
int    PASCAL  DsEInput      __((UNTYPE *stream));
void   PASCAL  DsEOutput     __((int c, UNTYPE *stream));

/***************** Prototype(s) for SCHNUM.C **************************/

int    PASCAL  DSmath        __((GLOBAL *));
CELP   PASCAL  DsCmpNumber   __((CELP, CELP, int));
USHORT PASCAL  DsGetNTag     __((CELP));

/***************** Prototype(s) for SCHERR.C **************************/

void   PASCAL  DsNoInitError __((CONST char *));
int    PASCAL  DsRetCode     __((void));
int    PASCAL  DsErrorLevel  __((int));
int    PASCAL  DsHandleError __((void));
int    CDECL   DsRepError    __((GLOBAL *, int, CELP));
void   PASCAL  DsClearError  __((void));
void   PASCAL  DsMemError    __((CONST char *));
void   PASCAL  DsStkError    __((void));
void   PASCAL  DsVError      __((int));
void   PASCAL  DsTypError    __((CELP));
void   PASCAL  DsError       __((int, CELP));
void   PASCAL  DsPError      __((int, PORT *));
char * PASCAL  DsErrMsg      __((int));
void   PASCAL  DsStrError    __((int, CONST char *));

/***************** Prototype(s) for SCHEXT.C **************************/

int    PASCAL  DSextend      __((GLOBAL *));
CELP   CDECL   Ds_globals    __((void));

/***************** Prototype(s) for SCHTRA.C **************************/

void   PASCAL  DsTraceInit   __((void));

/***************** Prototype(s) for SCHEVA.C **************************/

CELP   PASCAL  DsEval         __((CELP));
void   PASCAL  DsDefVar       __((CELP));

/***************** Prototype(s) for SCHSTR.C **************************/

void   PASCAL  DsIniBuf      __((int));
void   PASCAL  DsFreeBuf     __((char *));
void   PASCAL  DsBufDefrag   __((void));
int    PASCAL  DsFragmented  __((void));
CELP   PASCAL  DsStrCell     __((CONST char *));
CELP   CDECL   Ds_lstvec     __((CELP l));
CELP   PASCAL  DsGetStr      __((int n));
CELP   PASCAL  DsCmpVector   __((CELP a1,CELP a2));

/***************** Prototype(s) for SCHBI2.C **************************/

CELP    PASCAL DsStrBig  __((char *));
char  * PASCAL DsBigStr  __((CELP));
CELP    PASCAL big_cpy   __((CELP));
CELP    PASCAL big_add   __((CELP, CELP));
CELP    PASCAL big_sub   __((CELP, CELP));
CELP    PASCAL big_mul   __((CELP, CELP));
CELP    PASCAL big_div   __((CELP, CELP, CELP *));
CELP    PASCAL big_muli  __((ULONG, CELP));
void    PASCAL big_free  __((CELP));
int     PASCAL big_len   __((CELP));
CELP    PASCAL big_mul10 __((int, CELP));
int     PASCAL big_cmp   __((CELP, CELP));
CELP    PASCAL big_adds  __((CELP, CELP));
CELP    PASCAL big_subs  __((CELP, CELP));
CELP    PASCAL big_muls  __((CELP, CELP));
CELP    PASCAL big_divs  __((CELP, CELP, CELP *));
CELP    PASCAL int2big   __((CELP));
CELP    PASCAL real2big  __((CELP));
CELP    PASCAL big2int   __((CELP));
REAL    PASCAL big2real  __((CELP));
CELP    PASCAL makebig   __((CELP));
REAL    PASCAL makeflo   __((CELP));
REAL    PASCAL makeflo2  __((CELP, int));

/***************** Prototype(s) for SCHUTL.C **************************/

int    CDECL   DSutinit      __((GLOBAL *));

/***************** Prototype(s) for SCHTST.C **************************/

void   CDECL   main          __((int, char **));
void   PASCAL  DSGCmessage   __((int));

/***************** Prototype(s) for SCHINFO.C *************************/

void   PASCAL  info          __((void));
