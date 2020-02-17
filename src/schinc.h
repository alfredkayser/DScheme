/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHINC.H
**      SYSTEM   NAME:       SCHEME 
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/03/01
**
** DESCRIPTION: Global include file for the SCHEME system.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision: $
** CHANGER:     $Author: $
** WORKFILE:    $Workfile: $
** LOGFILE:     $Logfile: $
** LOGINFO:     $Log: $
**********************************************************************/
#define UNIX


#define MAXSTR    2048            /* Maximum string size */
#define IOBUFSIZE 512             /* buffer size of file IO ports */
#define AUTHOR    "Alfred Kayser"
#define SYSTEM    "DScheme"
#define VERSION   "1.5.5"

#if defined OS2 || defined MSDOS  /* All OS/2 compilers are more or less ANSI */
#define MSC                    /* Both are MicroSoft C (No TURBO-C Please!!!) */
#define __STDC__                   /* An ANSI volation itself, but it must be */
#endif

 /******************* All included headerfiles ******************/
#ifdef __STDC__
# include <signal.h>
# include <stdarg.h>
# include <stdlib.h>
# include <malloc.h>
# include <limits.h>
#else
# include <varargs.h>
# include <values.h>
# include <sys/signal.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <setjmp.h>

#ifndef OS2_INCLUDED               /* OS/2 has already defined these in os2.h */
  typedef          char  CHAR;
  typedef          short SHORT;
  typedef          long  LONG;
  typedef unsigned char  BYTE;
  typedef unsigned short USHORT;
  typedef unsigned long  ULONG;
# undef __                   /* Undefine various junk */
# undef PASCAL
# undef FAR
#endif
        
typedef SHORT WORD;         
typedef LONG DWORD;

#ifdef __STDC__
#define __(a) a
#define VAR_ARGS        
#define VAR_DCL
#define VAR_START(va,f) va_start(va,f)
#else
#define __(a) ()
#define VAR_ARGS        , va_alist
#define VAR_DCL         va_dcl
#define VAR_START(va,f) va_start(va)
#endif

#ifdef MSC
# pragma pack(2)
# include <conio.h>
# include <dos.h>
# define FAR               _far
# define FARMALLOC(size,t) _fmalloc((size)*sizeof(t))
# define FFREE(size)       _ffree(size)
# define CDECL             _cdecl
# define CONST             const
# define PASCAL           _fastcall
# define MATHTRAP         
# ifdef OS2
#  undef MSDOS             /* Undo 'bug' of Microsoft C */
# endif                    /* Compiling /Lp under OS2 has MSDOS defined */
#endif

#ifdef UNIX
# define UNTYPE    char
# define MATHTRAP         
#endif

/*---------------------- Defaults of various defines ----------------------*/
#ifndef INT_MAX
# define INT_MAX MAXINT
# define time_t  long
#endif
#ifndef SIG_ERR
# define SIG_ERR -1
#endif
#ifndef CLOCKS_PER_SEC
# define CLOCKS_PER_SEC   1000000L  /* number of ticks per seconds */
#endif
#ifndef FAR
# define FAR
# define FARMALLOC(size,t) (t *)malloc((size)*sizeof(t))
# define FFREE(size)       free(size)
#endif
#ifndef CDECL
# define CDECL
#endif
#ifndef PASCAL
# define PASCAL
#endif
#ifndef UNTYPE
# define UNTYPE void
#endif
#ifndef CONST
# define CONST
#endif
#ifndef SEEK_SET
# define SEEK_SET 0
# define SEEK_CUR 1
# define SEEK_END 2
#endif

/*----------------------------------------------------------------------*/
/* Definition datatypes                                                 */
/*----------------------------------------------------------------------*/

                                /*--------------------------------------*/
typedef struct  _cel    CEL;    /* Scheme cell                          */
typedef CEL  FAR *      CELP;   /* Pointer to scheme cell               */
typedef CELP FAR *      CELPP;  /* (far) pointer to cell pointer        */
typedef struct  _pts    PTS;    /* 2 pointers to Scheme cell            */
typedef struct  _pti    PTI;    /* 1 int & 1 pointer                    */
typedef struct  _tms    TMS;    /* 1 long & 1 float                     */
typedef struct  _str    STP;    /* String construction part             */
typedef union   _cvl    CVAL;   /* Scheme cell without tag              */
typedef struct  _key    KEY;    /* Key cell                             */
typedef struct  _ext    EXT;    /* External procedure cell              */
typedef struct  _big    BIG;    /* Bignum integer                       */
typedef int             CHR;    /* Scheme character                     */
typedef double          REAL;   /* Scheme floating point                */
typedef struct  _port   PORT;   /* Structure with filedefinition        */
typedef struct  _glb    GLOBAL; /* Structure with global var's          */
typedef struct _extdef  EXTDEF; /* External definition record           */
                                /*--------------------------------------*/
                 
typedef void  (PASCAL * TCALL) __((int));              /* Trace Call function */
typedef int   (PASCAL * INP_FUN) __((UNTYPE *));             /* same as fgetc */
typedef void  (PASCAL * OUT_FUN) __((int, UNTYPE *));        /* same as fputc */
typedef void  (PASCAL * CTL_FUN) __((UNTYPE *, int)); 
typedef CELP  (* EXTPROC) __((void));      
typedef int   (* ERRFUN)  __((GLOBAL *, int, CELP)); 

/*----------------------------------------------------------------------*/
/*  Scheme structures and such                                          */
/*  The CDR pointer used in _pts,_pti and _str must be on the           */
/*  same place in all structures.                                       */
/*----------------------------------------------------------------------*/

struct  _pts  {                 /*--------------------------------------*/
              CELP    cdr;      /* Cdr pointer                      (4) */
              CELP    car;      /* Car pointer                      (4) */
              };                /*--------------------------------------*/

struct  _pti  {                 /*--------------------------------------*/
              CELP    cdr;      /* Cdr pointer (blockallocation)    (4) */
              long    icar;     /* Car value (this pair is for)     (4) */
              };                /*--------------------------------------*/

struct  _ext  {                 /*--------------------------------------*/
              CELP    cdr;      /* Cdr pointer (points to name)     (4) */
              EXTDEF *extdef;   /* External function definition     (4) */
              };                /*--------------------------------------*/

struct  _key  {                 /*--------------------------------------*/
              CELP    cdr;      /* Cdr pointer (points to name)     (4) */
              WORD    keynr;    /* Keyword number                   (2) */
              WORD    args;     /* Number of arguments to expect    (2) */
              };                /*--------------------------------------*/

struct  _str  {                 /*--------------------------------------*/
              char *  stp;      /* Pointer to string                (4) */
              int     len;      /* length of string                 (2) */
              };                /*--------------------------------------*/

struct  _tms  {                 /*--------------------------------------*/
              WORD    time;     /* month+day (0=unknown)            (2) */
              WORD    date;     /* year (0=unknown)                 (2) */
              float   fsec;     /* float  (fraction of secs)        (4) */
              };                /*--------------------------------------*/

struct  _big  {                 /*--------------------------------------*/
              CELP    cdr;      /* ptr to next part of bignum.      (4) */
              long    int1;     /* 9 digits integer for bignum      (4) */
              };                /*--------------------------------------*/

union   _cvl  {                 /*--------------------------------------*/
              PTS     pair;     /* a pair cell (2 pointers)             */
              PTI     spec;     /* a special cell                       */
              KEY     key;      /* key cell                             */
              EXT     ext;      /* external procedure cell              */
              BIG     bignum;   /* bignumber                            */
              REAL    rv;       /* Floating point value                 */
              CHR     ch;       /* Character                            */
              TMS     ts;       /* Timestamp (long + float)             */
              STP     sp;       /* Pointer to string + length           */
              PORT   *prt;      /* Scheme port                          */
              };                /*--------------------------------------*/

struct  _cel  {                 /*--------------------------------------*/
              BYTE    tag;      /* Cell type indicator              (1) */
              BYTE    gcflags;  /* Garbage collector flags          (1) */
              CVAL    dat;      /* Value of this cell               (8) */
              };                /*--------------------------------------*/

struct _extdef{                 /*--------------------------------------*/
              char   *name;     /* pointer to name                  (4) */
              EXTPROC extrn;    /* function pointer                 (4) */
              WORD    args;     /* Number of arguments to expect    (2) */
              BYTE    at1;      /* Indicates type of arg 1 wanted   (1) */
              BYTE    at2;      /* Indicates type of arg 2 wanted   (1) */
              BYTE    at3;      /* Indicates type of arg 3 wanted   (1) */
              };                /*--------------------------------------*/

struct _port  {                 /*--------------------------------------*/
              UNTYPE *stream;   /* Pointer to stream info           (2) */
              OUT_FUN portout;  /* input function                   (2) */
              INP_FUN portin;   /* output function                  (2) */
              CTL_FUN portctl;  /* port control function            (2) */
              DWORD   lineno;   /* line number                      (4) */
              WORD    dir;      /* Type of port (-1=FREE,1=READ,..) (2) */
              CHR     unput;    /* Unputted character               (2) */
              };                /*--------------------------------------*/

struct  _glb  {                 /*--------------------------------------*/
              CELP   sysenv;    /* The system environment               */
              CELP   curenv;    /* Current environment in eval          */
              CELP   topexp;    /* Points to current expression in eval */
              CELP   curargs;   /* Pointer to list with arguments       */
              CELPP  conssym;   /* Array with pointers to constants     */
              CELPP  sstack;    /* Scheme eval. stack                   */
              CELPP  estack;    /* End of Scheme eval. stack            */
              CELPP  stkptr;    /* Pointer to top of stack              */
                                /*--------------------------------------*/
              WORD   hashsize;  /* Hashtable size (<32000)              */
              CELP  *hashtab;   /* hashtable                            */
              CELP   freel;     /* Pointer to first cell in freelist    */
              CELP   fblk;      /* Pointer to first block with cells    */
              long   freecels;  /* Number of free cells                 */
              CELP   freestr;   /* List with free strspace descriptors  */
              CELP   allostr;   /* List with allocaetd strspace blocks  */
              long   strspace;  /* number of characters free in strspace*/
              long   strsize;   /* total number of chars. in strspace   */
                                /*--------------------------------------*/
              DWORD  iobufflag; /* IObuffer flags, TRUE when used       */
              WORD   iobufsize; /* Size of IO buffers                   */
              WORD   ionumbuf;  /* Number of IO buffers                 */
              char  *iobuffers; /* Io buffer space                      */
              char  *bigbuf;    /* Big string buffer for IO             */
              WORD   bigbufsize;/* Big buffer size                      */
              DWORD  GCtreshold;/* Garbage collect when free<treshold   */
              WORD   GCflag;    /* Garbage Collect flag                 */
              char   prompt[40];/* prompt                               */
              WORD   evaldepth; /* Depth of evaluation                  */
              WORD   debug;     /* debug level                          */
              BYTE   verbose;   /* informative mode                     */
              BYTE   bignum;    /* Bignumber math linked?               */
                                /*--------------------------------------*/
              PORT  *inport;    /* Pointer to input port struct         */
              PORT  *outport;   /* Pointer to output port  "            */
              PORT  *errport;   /* Pointer to error port                */
              PORT  *sinport;   /* Pointer to stdin port (always active)*/
              PORT  *soutport;  /* Pointer to stdout port(always active)*/
              PORT  *serrport;  /* Pointer to stderr port(always active)*/
              PORT  *ports;     /* Port resource                        */
              WORD   prtnum;    /* Number of ports in port resource     */
                                /*--------------------------------------*/
              jmp_buf err_jmp;  /* Place to jump to on error            */
              int    errnr;     /* Error number                         */
              DWORD  errline;   /* Linenumber where error occurred      */
              CELP   errexp;    /* Expression in error                  */
              CELP   erritem;   /* Item in error                        */
              CELP   curexp;    /* Remember current expression          */
              ERRFUN errfunc;   /* Error reporting function             */
              WORD   magic;     /* A magic number. (should be last)     */
              };                /*--------------------------------------*/

#define COOKIE 21345            /* glo->magic must contain this value   */
#define BIGBASE 100000000L      /* Bignumber base */

/*--------------------------------------------------------------*/
/* The TYPE tag identifies the type of the car value of a cell  */
/*--------------------------------------------------------------*/
#define MASKCDR  (0x40)   /* bit mask for CDR pointer          */
#define MASKCAR  (0x20)   /* bit mask for CAR pointer          */
#define MASKSYM  (0x10)   /* bit mask to test for symbol (CDR) */
#define MASKNUM  (0x08)   /* bit mask to test for number       */
#define MASKVAR  (0x7E)   /* bit mask for env. vars */

#define _ISCDR(c) ((c)->tag & MASKCDR) 
#define _ISCAR(c) ((c)->tag & MASKCAR) 
#define _ISSYM(c) ((c)->tag & MASKSYM)
#define _ISNUM(c) ((c)->tag & MASKNUM) 
#define _ISBIG(c) (((c)->tag & (MASKCDR+MASKNUM))==MASKCDR+MASKNUM)
#define _ISVAR(c) (((c)->tag & 0x5E)==0x54)

                            /*------------------------------------------*/
#define TYPE_NIL  0x00      /*  NIL                                     */
#define TYPE_TMS  0x01      /*  Timestamp               (DWORD & float) */
#define TYPE_CHR  0x02      /*  Char                              (CHR) */
#define TYPE_PRT  0x03      /*  Scheme port           (pointer to PORT) */
#define TYPE_STR  0x04      /*  String             (str:length & char*) */
#define TYPE_VEC  0x05      /*  Vector type             (see: type_str) */
#define TYPE_OID  0x06      /*  OID                                     */
                            /*  Cells with NUMBERS:                     */
#define TYPE_INT  0x08      /*  Integer                         (DWORD) */
#define TYPE_FLT  0x09      /*  Floating point                   (REAL) */
                            /*  Cells with only a CDR pointer           */
#define TYPE_FREE 0x40      /*  Free cell                               */
                            /*  Cells with Numbers and CDR pointers     */
#define TYPE_BIGP 0x48      /*  BIG! Integer positive             (BIG) */
#define TYPE_BIGN 0x49      /*  BIG! Integer negative             (BIG) */
                            /*  Cells with Symbols:                     */
#define TYPE_EXT  0x50      /*  External procedure          (fun & int) */
#define TYPE_SPC  0x51      /*  Special pair                (constants) */
#define TYPE_FUN  0x52      /*  Function definition cell     (ptr+2int) */
#define TYPE_SYM  0x54      /*  Symbol                    (ptr to name) */
#define TYPE_KEY  0x55      /*  Keyword & std procs (2 WORD & ptr=>sym) */
                            /*  Cells with CAR and CDR pointers:        */
#define TYPE_PAIR 0x60      /*  Cel is a pair of pointers      (pts:2*) */
#define TYPE_PRC  0x61      /*  User defined proc   (pts to proc & env) */
                            /*  Cell ISSYM, ISCAR, ISCDR                */
#define TYPE_SYMD 0x74      /*  Symbol +value     (defined in toplevel) */
#define TYPE_MAC  0x75      /*  Macro definition           (2 pointers) */
                            /*------------------------------------------*/

 /**--------------------------------------------------------------**/
 /**  The IP number indicates what action should be taken when    **/
 /**  eval wants to evaluate this element.                        **/
 /**--------------------------------------------------------------**/
                         /**--------------------------------------**/
#define IP_EOF     0    /** EOF object                           **/
#define IP_DEF     1    /** Special form: DEFINE                 **/
#define IP_QUOTE   2    /** Special form: QUOTE                  **/
#define IP_LAMBDA  3    /** Special form: LAMBDA                 **/
#define IP_IF      4    /** Special form: IF                     **/
#define IP_SET     5    /** Special form: SET!                   **/
#define IP_BEGIN   6    /** Special form: BEGIN                  **/
#define IP_COND    7    /** Special form: COND                   **/
#define IP_CASE    8    /** Special form: CASE                   **/
#define IP_LET     9    /** Special form: LET                    **/
#define IP_LETA    10
#define IP_LETREC  11   /** Special form: LETREC                 **/
#define IP_MACRO   12   /** Special form: MACRO                  **/
#define IP_DO      13   /** Special form: DO                     **/
#define IP_DELAY   14   /** Special form: DELAY                  **/
#define IP_APPLY   15   /** Special form: APPLY                  **/
#define IP_EVAL    16   /** Special form: EVAL                   **/       
#define IP_CONS    17   /** Special CONS                         **/
#define IP_LIST    18   /** Special LIST                         **/
#define IP_NULL    19   /** Special NULL (and NOT)               **/
#define IP_NOT     20   /** Same as NULL                         **/
#define IP_AND     21   /** Special AND                          **/
#define IP_OR      22   /** Special OR                           **/
#define IP_EXIT    23
#define IP_BREAK   24
#define IP_VERSION 25
#define IP_COLLECT 26
                        /**--------------------------------------**/
#define IP_TRUE    27   /** not a real keyword **/
#define IP_INVIS   28
#define IP_UNDEF   29
#define IP_ELSE    30

 
 /********************** Trace defines **************************/
#define T_SELF    1
#define T_START   2
#define T_MACRO   3
#define T_END     4
#define T_TAIL    5
#define T_APPLY   6
#define T_LOOKUP  7

 /****************** Standard defines ***************************/
#define FREE     0x00
#define READMODE 0x01       /* bit 0 is read flag */
#define WRITMODE 0x02       /* bit 1 is write flag */
#define READWRIT 0x03       /* both 0 and 1 */
#define STANDARD 0x10       /* port is std-in/out/err */
#define UNPUTTED 0x20       /* char in unput buffer */
#define GCMARK   0x80       /* GC mark */

#include "scheme.h"
#include "schmac.h"
/*--------------------------------------------------------------*/
/* Include the SCH prototypes                                   */
/*--------------------------------------------------------------*/
#include "schdcl.h"

#ifdef FIXEDGLO
  extern GLOBAL DsGlo;        
#else
  extern GLOBAL *DsGlo;       
#endif

extern CELP Q_invis;
extern CELP Q_true;
extern CELP item;
extern CELP key;            /* Needed for tracing and sys-dumps */
