/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHEXT.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/10/21
**                                                                  
** DESCRIPTION: All the DScheme extended functions are defined here.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision$
** CHANGER:     $Author$
** WORKFILE:    $Workfile$
** LOGFILE:     $Logfile$
** LOGINFO:     $Log$
**********************************************************************/
#define DEBUG

#include "schinc.h"
#include "schdef.h"

STATIC CELP CDECL Ds_guenv      __((void));
STATIC CELP CDECL Ds_gsenv      __((void));
STATIC CELP CDECL Ds_gtimer     __((void));
STATIC CELP CDECL Ds_gtime      __((void));
STATIC CELP CDECL Ds_getsym     __((void));
STATIC CELP CDECL Ds_free       __((void));
STATIC CELP CDECL Ds_gettag     __((CELP arg));
STATIC CELP CDECL Ds_war        __((CELP p));
STATIC CELP CDECL Ds_err        __((CELP p));
STATIC CELP CDECL Ds_serr       __((CELP p));
STATIC CELP CDECL Ds_gpenv      __((CELP arg));
STATIC CELP CDECL Ds_strtime    __((CELP p));
STATIC CELP CDECL Ds_timestr    __((CELP p));
STATIC CELP CDECL Ds_nsecs      __((CELP p));
STATIC CELP CDECL Ds_ndays      __((CELP p));
STATIC CELP CDECL Ds_sprmpt     __((CELP arg));
STATIC CELP CDECL Ds_setgc      __((CELP arg));
STATIC CELP CDECL Ds_dumpcell   __((CELP arg));
STATIC CELP CDECL Ds_symbols    __((void));


static EXTDEF extensions[]=
    {
        {"GET-USER-ENV",    (EXTPROC)Ds_guenv,    0},
        {"GET-SYSTEM-ENV",  (EXTPROC)Ds_gsenv,    0},
        {"GET-TIME",        (EXTPROC)Ds_gtime,    0},
        {"DUMP-GLOBAL",     (EXTPROC)Ds_globals,  0},
        {"GET-SYMBOLS",     (EXTPROC)Ds_getsym,   0},
        {"GET-STATUS",      (EXTPROC)Ds_free,     0},
        {"GET-TIMER",       (EXTPROC)Ds_gtimer,   0},
        {"ERROR",           (EXTPROC)Ds_err,      1, 0},
        {"WARNING",         (EXTPROC)Ds_war,      1, 0},
        {"SERIOUS",         (EXTPROC)Ds_serr,     1, 0},
        {"GET-TAG",         (EXTPROC)Ds_gettag,   1, 0},
        {"GET-PROC-ENV",    (EXTPROC)Ds_gpenv,    1, TYPE_PRC},
        {"TIME->STRING",    (EXTPROC)Ds_timestr,  1, TYPE_TMS},
        {"STRING->TIME",    (EXTPROC)Ds_strtime,  1, TYPE_STR},
        {"NUMBER-OF-SECS",  (EXTPROC)Ds_nsecs,    1, TYPE_INT},
        {"NUMBER-OF-DAYS",  (EXTPROC)Ds_ndays,    1, TYPE_TMS},
        {"TIME->REAL",      (EXTPROC)DsTimeReal,  1, TYPE_TMS},
        {"REAL->TIME",      (EXTPROC)DsRealTime,  1, TYPE_FLT},
        {"SET-PROMPT!",     (EXTPROC)Ds_sprmpt,   1, TYPE_STR},
        {"SET-GC-TRESHOLD!",(EXTPROC)Ds_setgc,    1, TYPE_INT},
#ifdef DEBUG
        {"DUMP-CELL",       (EXTPROC)Ds_dumpcell, 1, 0},
        {"DUMP-SYMBOLS",    (EXTPROC)Ds_symbols,  0},
#endif
        ENDOFLIST
    };


/***************************************************************
** NAME:        DSextend                                   [API]
** SYNOPSIS:    int DSextend(glo)
**              GLOBAL *glo;
** DESCRIPTION: Initializes and link some extensions to the
**              DScheme system.
** RETURNS:     S_ERROR, if error occured.
**              S_OKAY otherwise.
***************************************************************/
                                                                             
int PASCAL DSextend(glo)
GLOBAL *glo;
{
    return(DSmultidef(glo,extensions));        /* link extended functions */
}


STATIC
CELP Ds_gettag(arg)
CELP arg;
{
    return DSINTCEL(TAG(arg));
}

STATIC
CELP Ds_war(arg)
CELP arg;
{
    DSERROR(ERRUSER1,arg);
    return NIL;
}


STATIC
CELP Ds_err(arg)
CELP arg;
{
    DSERROR(ERRUSER2,arg);
    return NIL;
}


STATIC
CELP Ds_serr(arg)
CELP arg;
{
    DSERROR(ERRUSER3,arg);
    return NIL;
}


STATIC
CELP Ds_gpenv(arg)
CELP arg;
{
    return CDRpart(CDRpart(arg));
}


STATIC
CELP Ds_setgc(arg)
CELP arg;
{
    GLOB(GCtreshold)=CELINT(arg);
    return arg;
}

STATIC
CELP Ds_guenv()
{
    return GLOB(curenv);
}

STATIC
CELP Ds_gsenv()
{
    return GLOB(sysenv);
}
     
STATIC
CELP Ds_gtimer()
{
    if (CLOCKS_PER_SEC>1000) {
        return DSINTCEL(clock()/(CLOCKS_PER_SEC/1000L));         /* returns milliseconds */
    } else {
        return DSINTCEL(clock()*((long)(1000L/CLOCKS_PER_SEC))); /* returns milliseconds */
    }
}


/***************************************************************
** NAME:        Ds_gtime
** SYNOPSIS:    CELP Ds_gtime()
** DESCRIPTION: Returns the current time.
** RETURNS:     The time.
***************************************************************/

STATIC
CELP Ds_gtime()
{
    return DsMakeTime(time(NULL));
}

STATIC
CELP Ds_strtime(p)
CELP p;
{
    return DsStrTime(STRPpart(p));
}

STATIC
CELP Ds_timestr(p)
CELP p;
{
    return DsStrCell(DsTimeStr(p));
}

STATIC
CELP Ds_nsecs(p)
CELP p;
{
    return DSINTCEL(60L * (LONG)CELTIM(p).time + (LONG)CELTIM(p).fsec);
}


STATIC
CELP Ds_ndays(p)
CELP p;
{
    return DSINTCEL(CELTIM(p).date);
}


STATIC
CELP Ds_sprmpt(arg)
CELP arg;
{
    strcpy(GLOB(prompt),STRPpart(arg));
    return(arg);
}


/***************************************************************
** NAME:        Ds_globals
** SYNOPSIS:    CELP Ds_globals()
** DESCRIPTION: This function prints the contents of the global
**              table to stdout.
** RETURN:      Q_invis
***************************************************************/
STATIC
CELP Ds_globals()
{
    CELP p;
    WORD i, c, c2, c3, c4, c5;
    REAL used, gemuse;
    LONG count, count2, total=sizeof(GLOBAL);
    char buf[255];

    count=0L;
    for (p=GLOB(fblk); ISTRUE(p); p=CDRpart(p)) count+=CARIpart(p);
    count+=IP_MAXCONST+1;
    used = (REAL)( 100L * (count-GLOB(freecels))) / (REAL)count ;
    gemuse=used;  /*1*/
    DsOuts(GLOB(errport),
                "***********************************************************************\n");
    DsOuts(GLOB(errport),
                "**  DUMP-GLOBAL          MEMORY  ENTRYSIZE    ENTRIES       FREE   USED\n");
    DsOuts(GLOB(errport),
                "**                    =========  =========  =========  =========  =====\n");
    sprintf(buf,"**  Global structure  %9u          -          -          -      -\n",
                              sizeof(GLOBAL));
    DsOuts(GLOB(errport),buf);
    sprintf(buf,"**  Cell space        %9lu  %9u  %9lu  %9lu  %4.1lf%%\n",
                              (LONG)sizeof(CEL)*count, sizeof(CEL), count,
                              GLOB(freecels), used);
    DsOuts(GLOB(errport),buf);
    total+=(LONG)sizeof(CEL)*count;
    count=0L;
    for (p=GLOB(freel); ISTRUE(p); p=CDRpart(p)) count++;
    if (count!=GLOB(freecels))
        DsOutf(GLOB(errport),"*** ERROR: %d free cells counted.\n",count);

    c=0;
    for (i=0;i<GLOB(hashsize);i++)
        if (ISTRUE(GLOB(hashtab)[i])) c++;
    used = (REAL) (100*c) / (REAL) GLOB(hashsize);
    gemuse+=used; /*2*/
    sprintf(buf,"**  Symbol table      %9u  %9u  %9u  %9u  %4.1lf%%\n",
                              (sizeof(*GLOB(hashtab)) * GLOB(hashsize)),
                              sizeof(*GLOB(hashtab)), GLOB(hashsize),
                              GLOB(hashsize)-c, used);
    DsOuts(GLOB(errport),buf);
    total+=(LONG)( sizeof(*GLOB(hashtab)) * GLOB(hashsize) );

    c2=c=0;
    for (i=0;i<GLOB(prtnum);i++)
    {
        if (GLOB(ports)[i].dir==FREE) c++;
        else c2++;
    }
    used = (REAL) (100*c2) / (REAL) GLOB(prtnum);
    gemuse+=used; /*3*/
    sprintf(buf,"**  Port table        %9u  %9u  %9u  %9u  %4.1lf%%\n",
                                (sizeof(PORT)*GLOB(prtnum)),
                                sizeof(PORT), GLOB(prtnum), c, used);
    DsOuts(GLOB(errport),buf);
    total+=(LONG)(sizeof(PORT)*GLOB(prtnum));

    count2=count=0L;
    c=0;
    for (p=GLOB(freestr); ISTRUE(p); p=CDRpart(p))
    {
        c++;
        count+=STRLpart(CARpart(p));
    }
    for (p=GLOB(allostr); ISTRUE(p); p=CDRpart(p))
        count2+=STRLpart(CARpart(p));
    if (GLOB(strsize>0))
    {
        used = (REAL) (100L*(GLOB(strsize)-GLOB(strspace))) / (REAL) GLOB(strsize);
        gemuse+=used; /*5*/
    }
    else used = 0.0;
    sprintf(buf,"**  String space      %9lu  %9u  %9u  %9lu  %4.1lf%%\n",
                              GLOB(strsize), 1, c, GLOB(strspace), used);
    DsOuts(GLOB(errport),buf);
    total+=GLOB(strsize);
    if (count2!=GLOB(strsize))
        DsOutf(GLOB(errport),"*** WARNING: total of string space blocks is %l\n",count2);
    if (count!=GLOB(strspace))
        DsOutf(GLOB(errport),"*** ERROR: %l free characters counted.\n",count);

    c=strlen(GLOB(bigbuf));
    if (BIGMAX)
        used = (REAL) (100*c) / (REAL) BIGMAX;
    else
        used = 0.0;
    gemuse+=used; /*6*/
    sprintf(buf,"**  String buffer     %9u  %9u  %9u  %9u  %4.1lf%%\n",
                                  BIGMAX, BIGMAX, 1, c, used);
    DsOuts(GLOB(errport),buf);
    total+=(LONG)(BIGMAX);

#ifdef MSDOS 
    c = FP_OFF(GLOB(estack))/sizeof(CELP) - FP_OFF(GLOB(sstack))/sizeof(CELP);
    c2 = FP_OFF(GLOB(stkptr))/sizeof(CELP) - FP_OFF(GLOB(sstack))/sizeof(CELP);
#else
    c = GLOB(estack)-GLOB(sstack);
    c2 = GLOB(stkptr)-GLOB(sstack);
#endif
    used = (c>0) ? (REAL) (100*c2) / (REAL) c : 0.0;
    gemuse+=used; /*7*/
    c3=sizeof(CELP);
    sprintf(buf,"**  Scheme stack      %9u  %9u  %9u  %9u  %4.1lf%%\n",
                                c3*c, c3, c, c - c2 , used); /* free */
    DsOuts(GLOB(errport),buf);
    total+=(LONG)(sizeof(*GLOB(stkptr))*c);

    DsOuts(GLOB(errport),
                "**                    =========\t\t\t\t\t  =====\n");
    sprintf(buf,"**  Total             %9lu\t\t\t\t\t  %4.1lf%%\n**\n",total, gemuse/7.0);
    DsOuts(GLOB(errport),buf);

    c=c2=c3=c4=c5=0;
    for (i=0;i<GLOB(hashsize);i++)
    {
        for (p=GLOB(hashtab[i]); ISTRUE(p); p=CDRpart(p))
            switch(TAG(CARpart(p)))
            {
            case TYPE_SYM:c++;break;    /* symbols */
            case TYPE_EXT:c2++;break;   /* externals */
            case TYPE_PRC:c3++;break;   /* lambda's */
            case TYPE_MAC:c4++;break;   /* macro's */
            case TYPE_KEY:c5++;break;   /* keyword (const) */
            }
    }
    DsOutf(GLOB(errport),"**  Tokens: %d symbols, %d consts, %d macros, %d lambda's, %d externals\n",c,c5,c4,c3,c2);
    DsOutf(GLOB(errport),"**  Error code: %d, item: %a, expression %a\n", GLOB(errnr), GLOB(erritem), GLOB(errexp));
    DsOutf(GLOB(errport),"**  GarbageCollect flag: %d, treshold: %l\n", GLOB(GCflag), GLOB(GCtreshold));
    DsOuts(GLOB(errport),"***********************************************************************\n");
    return (Q_invis);
}


#ifdef DEBUG
STATIC CELP Ds_dumpcell(p)
CELP p;
{
    fprintf(stderr,"* Address: %lp\n",p);
    if (ISTRUE(p))
    {
        fprintf(stderr,"* TAGpart: %02x\n",TAGpart(p));
        fprintf(stderr,"* GCflags: %02x\n",GCFpart(p));
        fprintf(stderr,"* HexDump: %08lx%08lx\n",INTpart(p),CDRpart(p));
        if (ISCDR(p))
            Ds_dumpcell(CDRpart(p));
    }
    return (Q_invis);
}


STATIC CELP Ds_symbols()
{
    CELP p;
    int i;

    for (i=0;i<GLOB(hashsize);i++)
    {
        if (p=GLOB(hashtab[i]))
        {
            fprintf(stderr,"* [%4d] :", i);
            for (; ISTRUE(p); p=CDRpart(p))
                fprintf(stderr," %s", STRPpart(CDARpart(p)));
            fprintf(stderr,"\n");
        }
    }
    return (Q_invis);
}
#endif


/***************************************************************
** NAME:        Ds_getsym
** SYNOPSIS:    CELP Ds_getsym()
** DESCRIPTION: This function gets the contents of the symbol
**              table to GLOB(errport). GetSym is defined for
**              debugging purposes.
** RETURNS:     List of interned symbols.
***************************************************************/
STATIC
CELP Ds_getsym()
{
    int i;
    CELP p;
    CELP n=NIL;

    for (i=0; i<GLOB(hashsize); i++)
        for (p=GLOB(hashtab[i]);ISTRUE(p);p=CDRpart(p))
            n=DsCons(CARpart(p),n);
    return(n);
}


/***************************************************************
** NAME:        Ds_free
** SYNOPSIS:    CELP Ds_free()
**              CELP cfname;    pointer to cel with fname.
** DESCRIPTION: Returns the number of free cels, and the number
**              of cels.
** RETURNS:     A list containing two numbers.
***************************************************************/
STATIC
CELP Ds_free()
{
    CELP p;
    LONG count;

    count=0L;
    for (p=GLOB(fblk); ISTRUE(p); p=CDRpart(p))
        count+=CARIpart(p);                     /* total cells */
    return DsCons(DSINTCEL(count+IP_MAXCONST),DSINTCEL(GLOB(freecels)));
}

