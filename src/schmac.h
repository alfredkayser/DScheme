/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHMAC.H
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/02/03
**
** DESCRIPTION: Here all the macros defined.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schmac.h  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHMAC.H_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHMAC.H_V  $
**
**                 Rev 1.0   12 Oct 1989 11:43:42   JAN
**              Initial revision.
**********************************************************************/
#define ISNIL(p)    ((p)==NIL)
#define ISTRUE(p)   ((p)!=NIL)

#ifndef FIXEDGLO
# define GLOB(x)    (DsGlo->x)
# define SETGLOB(x,f) if (ISNIL(x)||(x->magic!=COOKIE)) DsNoInitError(f);\
                    DsGlo=x
# define PGLOBAL    DsGlo
#else
# define GLOB(x)    (DsGlo.x)
# define SETGLOB(x,f) if (ISNIL(x)||(x->magic!=COOKIE)) DsNoInitError(f);
# define PGLOBAL    &DsGlo
#endif


/* The following macro tricks makes the compiler believe that   */
/* the error functions never stops.  This results in a better   */
/* optimization.                                                */
/* The AIX xlc compiler has an internal bug when 'while(1) e;' is used!! */
#ifdef AIX
#define STOP(e)      while(e,1);
#else
#define STOP(e)      do{e;while(1);}while(0)
#endif
#define DSERROR(n,p)  STOP(DsError(n,p))
#define DSPERROR(n,p) STOP(DsPError(n,p))
#define DSSERROR      STOP(DsStkError())
#define DSVERROR(n)   STOP(DsVError(n))
#define DSTERROR(p)   STOP(DsTypError(p))
#define DSTERROR(p)   STOP(DsTypError(p))

/*=====================================IO MACROS==============================*/
#define DsOut(p,c) (p)->portout((c),(p)->stream)

#define INPUT      DsInput(GLOB(inport))
#define UNPUT(c)   DsUnput(GLOB(inport),c)
#define OUTPUT(c)  DsOut(GLOB(outport),c)
#define NEWLINE    OUTPUT('\n')

/*=====================================CEL HANDLING===========================*/
#define CARIpart(l)  ((l)->dat.spec.icar)
#define PROCpart(l)  ((l)->dat.ext.extrn)
#define KEYpart(l)   ((l)->dat.key.keynr)
#define ARGpart(l)   ((l)->dat.key.args)
#define TAGpart(l)   ((l)->tag)
#define GCFpart(l)   ((l)->gcflags)

/*======================================FAST CEL HANDLING=====================*/
#define TAG(l)          (ISNIL(l)?TYPE_NIL:(l)->tag)
#define CARpart(l)      ((l)->dat.pair.car)
#define CDRpart(l)      ((l)->dat.pair.cdr)
#define CAARpart(l)     (((l)->dat.pair.car)->dat.pair.car)
#define CADRpart(l)     (((l)->dat.pair.cdr)->dat.pair.car)
#define CDARpart(l)     (((l)->dat.pair.car)->dat.pair.cdr)
#define CDDRpart(l)     (((l)->dat.pair.cdr)->dat.pair.cdr)
#define CAAARpart(list) (CARpart(CAARpart(list)))
#define CAADRpart(list) (CARpart(CADRpart(list)))
#define CADARpart(list) (CARpart(CDARpart(list)))
#define CADDRpart(list) (CARpart(CDDRpart(list)))
#define CDAARpart(list) (CDRpart(CAARpart(list)))
#define CDADRpart(list) (CDRpart(CADRpart(list)))
#define CDDARpart(list) (CDRpart(CDARpart(list)))
#define CDDDRpart(list) (CDRpart(CDDRpart(list)))

/*====================================FAST STACK HANDLING=====================*/
#define PUSH(p)         if (GLOB(stkptr)>=GLOB(estack)) DSSERROR;\
                        *GLOB(stkptr)++=(p)
#define STKADD(n)       if ((GLOB(stkptr)+=(n))>=GLOB(estack)) DSSERROR
#define POP             (--GLOB(stkptr))        /* don't check on empty stack */
#define STKARG(n)       (*(GLOB(stkptr)-(n)))  /* g/set n-th value from stack */

/*====================================ACCELLERATORS===========================*/
#define ISTYP(p,typ)    (ISTRUE(p) && TAGpart(p)==typ)
#define ISNUM(p)        (ISTRUE(p) && _ISNUM(p))
#define ISCDR(p)        (ISTRUE(p) && _ISCDR(p))
#define ISCAR(p)        (ISTRUE(p) && _ISCAR(p))
#define ISSYM(p)        (ISTRUE(p) && _ISSYM(p))
#define ISBIG(p)        (ISTRUE(p) && _ISBIG(p))

#define ISPAIR(p)       (ISTRUE(p) && (p)->tag==TYPE_PAIR)
#define ISKEY(p)        (ISTRUE(p) && (p)->tag==TYPE_KEY)
#define ISATOM(p)       (ISTRUE(p) && (p)->tag!=TYPE_PAIR)
#define ISFALS(p)       ISNIL(p)
#define INITCEL(t,n)    (t=DsGetCell(n))
#define LIST(a,b)       (DsCons(a,DsCons(b,NIL)))
#define TESTNUM(p)      if (!ISNUM(p)) DSTERROR(p)
#define TYPCHECK(p,typ) if (ISNIL(p)||(TAGpart(p)!=typ)) DSTERROR(p);
#define NUMCHECK(exp,n) if (DsLength(exp)<n) DSERROR(ERRARC,exp)
#ifdef MATHTRAP
#define ZEROCHECK(p)    p
#else
#define ZEROCHECK(p)    if (FLTpart(p)==0.0) DSVERROR(ERRDIV0)
#endif

/*=======================DATA ABSTRACTION MACROS==============================*/
#define BIGBASE         100000000L
#define STRPpart(c)     ((c)->dat.sp.stp)
#define STRLpart(c)     ((c)->dat.sp.len)
#define VECPpart(c)     ((CELP *)STRPpart(c))
#define VECLpart(c)     (STRLpart(c)/sizeof(CELP))
#define BIGpart(c)      ((c)->dat.bignum.int1)
#define INTpart(c)      ((c)->dat.bignum.int1)
#define CHRpart(c)      ((c)->dat.ch)
#define FLTpart(c)      ((c)->dat.rv)
#define PRTpart(c)      ((c)->dat.prt)

#ifdef INLINE
extern CELP _tmp_p;
#define DSINTCEL(v)     (INTpart(_tmp_p=DsGetCell(TYPE_INT))=v),_tmp_p
#define DSFLTCEL(v)     (FLTpart(_tmp_p=DsGetCell(TYPE_FLT))=v),_tmp_p
#define DSCHRCEL(v)     (CHRpart(_tmp_p=DsGetCell(TYPE_CHR))=v),_tmp_p
#define DSPRTCEL(v)     (PRTpart(_tmp_p=DsGetCell(TYPE_PRT))=v),_tmp_p
#define INTCEL(c,v)     (INTpart(c=DsGetCell(TYPE_INT))=v)
#define FLTCEL(c,v)     (FLTpart(c=DsGetCell(TYPE_FLT))=v)
#define PRTCEL(c,v)     (PRTpart(c=DsGetCell(TYPE_PRT))=v)
#define CHRCEL(c,v)     (CHRpart(c=DsGetCell(TYPE_CHR))=v)
#else
#define DSINTCEL(v)     DsIntCell(v)
#define DSFLTCEL(v)     DsFltCell((double)v)
#define DSCHRCEL(v)     DsChrCell(v)
#define DSPRTCEL(v)     DsPrtCell(v)
#define INTCEL(c,v)     c=DsIntCell(v)
#define FLTCEL(c,v)     c=DsFltCell(v)
#define CHRCEL(c,v)     c=DsChrCell(v)
#define PRTCEL(c,v)     c=DsPrtCell(v)
#endif
#define DSSTRCEL(v)     DsStrCell(v)
#define DSTMSCEL(v)     DsStrTime(v)
#define STRCEL(c,v)     c=DsStrCell(v)
#define TMSCEL(c,v)     c=DsStrTime(v)

#define CELINT(c)       BIGpart(c)                     /* assume short bignum */
#define CELPRT(c)       c->dat.prt
#define CELFLT(c)       c->dat.rv
#define CELCHR(c)       c->dat.ch
#define CELTIM(c)       c->dat.ts
#define CELTMS(c)       DsTimeStr(c)
#define CELEXT(c)       c->dat.ext.extdef

/*===========================TIME HANDLING====================================*/
#define COM_TIME(h,m)   (((h) << 8) +(m))
#define TIME_H(t)       (((t) >> 8) & 0xFF)
#define TIME_M(t)       ((t) & 0xFF)

/*----------------------------------------------------------------------------*/
/* The following macros return a pointer to the cel containing the            */
/* corresponding IP number.                                                   */
/*----------------------------------------------------------------------------*/
#define CSYM(n)      (GLOB(conssym)[n])
#define Q_false      (NIL)          /* Boolean FALSE is translated into a nil */
#define Q_lambda     (CSYM(IP_LAMBDA))
#define Q_undef      (CSYM(IP_UNDEF))
#define Q_quote      (CSYM(IP_QUOTE))
#define Q_else       (CSYM(IP_ELSE))
#define Q_eof        (CSYM(IP_EOF))

#define TEST(x)       ((x)?Q_true:Q_false)
#define RETBOO(x)     return TEST(x)
#define BIGMAX        (GLOB(bigbufsize))            /* Maximum bigstring size */
#define BIGBUF        (GLOB(bigbuf))                     /* bigstring pointer */
#define TRACELEVEL    (GLOB(trace))
#define PROTECT(p)    (p)->gcflags|=0x2;

#ifdef UNIX
# define GETMEM(p,t,s,m) if ((p=(t *)malloc(s))==NULL) DsMemError(m)
#else
# define GETMEM(p,t,s,m) if ((p=malloc(s))==NULL) DsMemError(m)
#endif

#ifndef MSC
# define min(a,b) (a)<(b)?(a):(b)
# ifndef labs
#  define labs(a) (a)<0?-(a):(a)
# endif
#endif

#define BELL '\007'
#define ENDOFLIST {NULL}
