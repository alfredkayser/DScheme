/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHEME.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/05
**
** DESCRIPTION: Main module for the SCHEME system. Contains Scheme
**              starter, caf‚ and waiters. This module contains
**              also the error function(s) and the memory
**              functions.
** HISTORY:     Version:     1.1
**              Completion:  89/03/21
**              Author:      Alfred Kayser
**              Changes:     Added new-caf‚ & waiter, eval-stack,
**                           call/cc. Replaced yac and lex by (very)
**                           fast code in schrdr.c
**              Version:     1.2
**              Completion:  89/05/25
**              Author:      Alfred Kayser
**              Description: Added Garbage Collector, Kernel functions
**                           'precompiled', compiled in small model
**                           with far pointers to cells and S-stack.
**                           Completed kernel functions. Rewrite of
**                           initglobal. Cleanup error handling, des-
**                           ripting string now outside the scheme.lib
**                           Rewritten timestamp functions.
**              Version:     1.3
**              Completion:  89/05/25
**              Author:      Alfred Kayser
**              Description: Some beauty changes. Added macro system.
**                           Added string space. Debugged GC.
**              Version:     1.4
**              Completion:  89/08/16
**              Author:      Alfred Kayser
**              Description: Cleaned up API interface.
**                           Removed some bugs from the SCHBIG module.
**              Version:     1.5
**              Completion:  89/11/11
**              Author:      Alfred Kayser
**              Description: Cleaned interface for external definitions
**                           New TAG numbering, implemented LET,LET*
**                           LETREC and DO. Renamed all function names
**                           according new DNPAP standards.
**              Version:     1.5.3
**              Completion:  90/10/26
**              Author:      Alfred Kayser
**              Description: Small and little cleanups.
**                           Found some ways to optimize code in both
**                           speed and size.
**              Version:     1.5.4
**              Completion:  90/11/09
**              Author:      Alfred Kayser
**              Description: Small and little cleanups.
**                           Found again some ways to optimize code
**                           in both speed and size.
**                           Removed bug introduced by 1.5.3:
**                           Argument count check wrong.
**                           Defined new functions: Append & Append!
**              Version:     1.5.5
**              Completion:  91/03/08
**              Author:      Alfred Kayser
**              Description: Used one Cons less in argument lists.
**                           Define now also works at the beginning
**                           of the body. The one cons less means about
**                           10% speed improvement!!
***********************************************************************
** PVCS INFORMATION **
**********************
** REVISION:    $Revision: 1.5.5  $
** CHANGER:     $Author:   ALFRED  $
** WORKFILE:    $Workfile: $
** LOGFILE:     $Logfile:  $
** LOGINFO:     $Log:      $
**********************************************************************/
#include "schinc.h"
#include "schprc.h"

#ifdef FIXEDGLO
 GLOBAL DsGlo;
#else
 GLOBAL *DsGlo;         /* The only global variable in this system */
#endif                  /* contains all other globals.             */

CELP Q_true;
CELP Q_invis;
CELP item;

#ifdef INLINE
CELP _tmp_p
#endif

int toupper __((int c));
STATIC GLOBAL * PASCAL DsNewGlobal  __((void));
STATIC void PASCAL DsIniEStack      __((int));
STATIC void PASCAL DsIniEnvironment __((void));
STATIC void PASCAL DsIniHash        __((int));
STATIC void PASCAL DsSetStdPorts    __((PORT *, PORT *, PORT *));
STATIC void PASCAL DsIniKey         __((CELP, int, int, char *));
STATIC void PASCAL DsIniConst       __((void));
STATIC void CDECL  DsCtrlHandler    __((int));

/***************************************************************
** NAME:        DSinit                                     [API]
** SYNOPSIS:    int DSinit(Gloptr, {codenr, val,} 0 );
**              GLOBAL **gloptr pointer to the global struct.
**              int    codenr;  Argument code nr
**              ???    val;     Argument value (can be of any
**                              type.
** DESCRIPTION: DSinit reads and evaluates the data and programs
**              found in the INPORT and puts the results in the
**              OUTPORT. INPORT and OUTPORT are defined via the
**              PORT functions. These can be strings or files.
**
**              Arguments are passed in a special way. It is not
**              needed to pas all arguments.
**
**              Codenr:      Type:   Description:
**               S_INPORT    *PORT   Pointer to input port
**               S_OUTPORT   *PORT   Pointer to output port
**               S_NUMCEL    int     Number of cells to allocate
**               S_HASHSIZE  int     Size of hashtable (should
**                                   be a prime.)
** RETURNS:     S_ARGERR, if a error in the arguments was found.
**              S_ERROR, if a DScheme error was signalled.
**              S_OKAY, otherwise.
** SEE ALSO:    DSfopen, DSinitport
***************************************************************/
extern EXTDEF extensions[];
int DSinit(global VAR_ARGS)
GLOBAL ** global; 
VAR_DCL
{
    unsigned int val, hashsize, numport, stack, strings,
                 numbuf, bufsize, bigbuf, verb;
    LONG numcel;
    va_list va;
    PORT *inport,*outport,*errport;
    char *prompt=NIL;
    ERRFUN errfun=NIL;

#ifndef FIXEDGLO
    DsGlo = *global;
#else
    global;         /* not used */
#endif
    bufsize=IOBUFSIZE;
    if (PGLOBAL==NULL || GLOB(magic)!=COOKIE)          /* Not yet initialized */
    {
        numcel=6550L;                         /* begin to allocate 6550 cells */
        strings=10000;
        stack=10000;
        hashsize=997;                                              /* initial */
        bigbuf=2048;
        numport=8;
        numbuf=4;
        prompt="\nDS>";
    }
    else                               /* restart: don't allocate extra cells */
    {
        numbuf=stack=strings=hashsize=numport=bigbuf=0;
        numcel=0L;
    }
    inport=NIL;                                                   /* defaults */
    outport=NIL;
    errport=NIL;
    verb = -1;

    VAR_START(va,global);
    while ((val = va_arg(va,int))!=0)                     /* handle arguments */
    {
         switch(val)
         {
         case S_INPORT :  inport   = va_arg(va, PORT *); break;
         case S_OUTPORT:  outport  = va_arg(va, PORT *); break;
         case S_ERRPORT:  errport  = va_arg(va, PORT *); break;
         case S_STDIN  :  inport   = GLOB(sinport);      break;
         case S_STDOUT :  outport  = GLOB(soutport);     break;
         case S_STDERR :  errport  = GLOB(serrport);     break;
         case S_STRING :  strings  = va_arg(va, int);    break;
         case S_STACK  :  stack    = va_arg(va, int);    break;
         case S_NUMPORT:  numport  = va_arg(va, int);    break;
         case S_NUMCEL :  numcel   = va_arg(va, LONG);    break;
         case S_HASHSIZE: hashsize = va_arg(va, int);    break;
         case S_BIGBUF :  bigbuf   = va_arg(va, int);    break;
         case S_BUFFERS:  numbuf   = va_arg(va, int);
                          bufsize  = va_arg(va, int);    break;
         case S_PROMPT :  prompt   = va_arg(va, char *); break;
         case S_VERBOSE:  verb     = va_arg(va, int);    break;
         case S_CTRLC  :  if (signal(SIGINT,DsCtrlHandler)==SIG_ERR)
                               return(S_ERROR);
                          break;
         case S_ERRFUN :  errfun   = va_arg(va, ERRFUN); break;
         default: return(S_ARGERR);                     /* error in arguments */
         }
    }
    va_end(va);

    if (PGLOBAL==NIL || GLOB(magic)!=COOKIE)           /* Not yet initialized */
    {
#ifndef FIXEDGLO
        DsGlo=DsNewGlobal();
        *global=DsGlo;                                /* make known to caller */
#else
        DsNewGlobal();
        *global=&DsGlo;                               /* make known to caller */
#endif
        if (errfun) GLOB(errfunc)=errfun;
        if (!GLOB(errfunc)) GLOB(errfunc)=DsRepError;/* Default error handler */
        if (setjmp(GLOB(err_jmp)))  /* If error somewhere in init jmp to here */
        {
            DsHandleError();
            return(S_ERROR);
        }
        DsIniPorts(numport,bigbuf,numbuf,bufsize);
        DsStdPorts();                            /* initialize standard ports */
        DsIniEStack(stack);
        DsIniParser();
        DsIniHash(hashsize);
        DsIniCells(numcel);
        DsIniBuf(strings);
        DsIniEnvironment();
        DsTraceInit();
    }
    else
    {
        if (setjmp(GLOB(err_jmp)))  /* If error somewhere in init jmp to here */
        {
            DsHandleError();
            return(S_ERROR);
        }
        if (numcel) DsIniCells(numcel);                /* allocate more cells */
        if (strings) DsIniBuf(strings);           /* resize more string space */
    }
    if (prompt!=NIL) strcpy(GLOB(prompt),prompt);
    DsSetStdPorts(inport,outport,errport);
    if (verb!=-1) GLOB(verbose)=(BYTE)verb;
    return(S_OKAY);
}



/***************************************************************
** NAME:        DSclose                                    [API]
** SYNOPSIS:    int DSclose(glo)
**              GLOBAL **glo     pointer to the environment.
** DESCRIPTION: DSclose closes all open I/O ports.
**              It is not considered a serious error if glo is
**              NIL.
** RETURNS:     S_ERROR, if global is NIL.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSclose(global)
GLOBAL **global;
{
    int i;
    CELP p,q;

#ifndef FIXEDGLO
    DsGlo = *global;
#endif
    signal(SIGINT,SIG_DFL);            /* restore interrupt handler */
    if (PGLOBAL!=NIL)                  /* global exist? */
    {
        for (i=0;i<GLOB(prtnum);i++)   /* close all ports */
            DsClosePort(GLOB(ports+i));
        /* ... free all allocated memory ... */
        /* Free all string space blocks */
        for (q=GLOB(allostr); ISTRUE(q); q=CDRpart(q))
            free(STRPpart(CARpart(q))); /* free this block */

        /* Free all cell blocks */
        q=GLOB(fblk);
        while (ISTRUE(q))
        {
            p=CDRpart(q);                /* get pointer to next block */
            FFREE(q);                    /* free this block */
            q=p;                         /* point to next block */
        }
        FFREE(GLOB(sstack));
        free(GLOB(hashtab));
        free(GLOB(bigbuf));
        free(GLOB(ports));
#ifndef FIXEDGLO	
        free(DsGlo);
        *global=DsGlo=NIL;
#endif
        return(S_OKAY);
    }
    else
        return(S_ERROR);
}


/***************************************************************
** NAME:        DScheme                                    [API]
** SYNOPSIS:    int DScheme(global);
**              GLOBAL *global     pointer to the environment.
** DESCRIPTION: Scheme reads and evaluates the data and programs
**              found in the INPORT and puts the results in the
**              OUTPORT.
** RETURNS:     S_ERROR, if error occured in DScheme.
**              S_END, otherwise (even if "exit" is evaluated).
***************************************************************/
int PASCAL DScheme(global)
GLOBAL * global;                         /* pointer to the global environment */
{
    SETGLOB(global,"DScheme");                 /* test and set global pointer */
    return DsWaiter();                                        /* start waiter */
}


/***************************************************************
** NAME:        DSextdef                                   [API]
** SYNOPSIS:    int DSextdef(glo,extdef)
**              GLOBAL *glo     pointer to the environment.
**              EXTDEF *extdef  external procedure def record.
** DESCRIPTION: Links an external (user supplied) C-function in
**              the DScheme environment.
** RETURN:      S_ERROR, if an error occured.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSextdef(global, extdef)
GLOBAL *global;
EXTDEF *extdef;
{
    CELP ext;

    SETGLOB(global,"DSextdef");                /* test and set global pointer */
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    ext=DsInsExt(strupr(extdef->name));
    TAGpart(ext)=TYPE_EXT;
    CELEXT(ext)=extdef;
    return(S_OKAY);
}


/***************************************************************
** NAME:        DSmultidef                                 [API]
** SYNOPSIS:    int DSmultidef(glo,extdef)
**              GLOBAL *glo     pointer to the environment.
**              EXTDEF *extdef  array of external procedures.
** DESCRIPTION: Links multiple external (user supplied)
**              C-functions in the DScheme environment.
** RETURN:      S_ERROR, if an error occured.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSmultidef(glo,extens)
GLOBAL *glo;
EXTDEF *extens;
{
    SETGLOB(glo,"DSmultidef");                 /* test and set global pointer */
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    DsFuncDef(extens);
    return(S_OKAY);
}


/***************************************************************
** NAME:        DsFuncDef
** SYNOPSIS:    void DsFuncDef(extdef)
**              EXTDEF *extdef  array of external procedures.
** DESCRIPTION: Links multiple external (user supplied)
**              C-functions in the DScheme environment.
**              Internal Use Only.
** RETURN:      void
** SEE:         DSmultidef
***************************************************************/

void PASCAL DsFuncDef(extens)
EXTDEF *extens;
{
    CELP ext;
    while (extens->name!=NULL)
    {
        ext=DsInsExt(strupr(extens->name)); /* insert name in symbol table */
        TAGpart(ext)=TYPE_EXT;
        CELEXT(ext)=extens++;               /* define it */
    }
}


/***************************************************************
** NAME:        DSeval                                     [API]
** SYNOPSIS:    int DSeval(glo,object, result)
**              GLOBAL *glo;    pointer to the environment.
**              CELP object;    object to evaluate
**              CELP **result;  result value
** DESCRIPTION: Evaluates a DScheme object in the environment
**              pointed to by glo.
** RETURN:      S_ERROR, if an error occured.
**              S_END, if the DScheme 'exit' is evaluated.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSeval(global,object,presult)
GLOBAL *global;
CELP object;
CELP *presult;
{
    SETGLOB(global,"DSeval");                 /* test and set global pointer */
    *presult=NIL;
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    *presult=DsEval(object);
    return(S_OKAY);
}


/***************************************************************
** NAME:        DSread                                     [API]
** SYNOPSIS:    int DSread(glo, port, result)
**              GLOBAL *glo;    pointer to the environment.
**              PORT *port;     Port to read from
**              CELP **result;  result value
** DESCRIPTION: Reads a DScheme object in the environment
**              pointed to by glo. if <port> is NULL, the read
**              will be done from glo->inport.
** RETURN:      S_ERROR, if an error occured.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSread(global,port,presult)
GLOBAL *global;
PORT *port;
CELP *presult;
{
    SETGLOB(global,"DSread");                  /* test and set global pointer */
    *presult=NIL;
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    *presult=DsRead(port?port:GLOB(inport));
    return(S_OKAY);
}



/***************************************************************
** NAME:        DSinitport                                 [API]
** SYNOPSIS:    int DSinitport(global, pport, mode, stream,
**                             inputp, outputp, closep)
**              GLOBAL *global;
**              PORT **port;    Pointer to new port.
**              int mode;
**              void *stream;
**              INP_FUN inputp;
**              OUT_FUN outputp;
**              CLO_FUN closep;
** DESCRIPTION: Request for a IO port.
**              Finds an empty port slot in the global port
**              resources and initializes it.
**              Calls noinit_error when global is NIL.
** RETURNS:     S_ERROR, if an error occured.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSinitport(global,pport,mode,stream,inputp,outputp,closep)
GLOBAL *global;
PORT **pport;
int mode;
UNTYPE *stream;
INP_FUN inputp;
OUT_FUN outputp;
CTL_FUN closep;
{
    SETGLOB(global,"DSinitport");              /* test and set global pointer */
    *pport=NIL;
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    *pport = (PORT *)DsInitPort(mode,stream,inputp,outputp,closep);
    return(S_OKAY);
}


/***************************************************************
** NAME:        DSfopen                                    [API]
** SYNOPSIS:    int DSfopen(global, pport, name, mode);
**              GLOBAL *global;
**              PORT **pport;   pointer to new port.
**              char *name;     name of file to be opened.
**              int   mode;     filemode
** DESCRIPTION: Opens a SCHEME port (standard file port)
**              mode can be READMODE, WRITMODE or READWRIT.
**              Calls noinit_error when global is NIL.
** RETURNS:     S_ERROR, if an error occured.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DSfopen(global, pport, fname, mode)
GLOBAL *global;
PORT **pport;
CONST char *fname;
int   mode;
{
    SETGLOB(global,"DSfopen");                 /* test and set global pointer */
    *pport=NIL;
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    *pport=DsFOpen(fname,mode);
    return(S_OKAY);
}


/***************************************************************
** NAME:        DsWaiter
** SYNOPSIS:    int DsWaiter();
** DESCRIPTION: This function is called by the Scheme system
**              to Read-Eval-Print each element of the input
**              stream and print it at the output
** RETURNS:     S_END, end of REP loop,
**              S_ERROR error occurred.
***************************************************************/
int CDECL DsWaiter()
{
    CELPP   oldstk;
    CELP    curenv;
    jmp_buf oldjmp;
    int     level=0;

#ifdef GCPORTS
    PUSH( DsCons( DSPRTCEL(GLOB(errport)),          /* keep ports save for GC */
                  DsCons( DSPRTCEL(GLOB(inport)),
                          DSPRTCEL(GLOB(outport)))));
#endif
    curenv = GLOB(curenv);
    oldstk = GLOB(stkptr);
    memcpy (oldjmp,GLOB(err_jmp),sizeof(oldjmp));      /* save return address */
    setjmp (GLOB(err_jmp));                /* return here when error occurres */
    if (GLOB(errnr)!=NOERROR)
    {
        GLOB(stkptr) = oldstk;                         /* reset stack pointer */
        if (GLOB(errnr)==ERRXIT)
            level=1;                                         /* a normal exit */
        else
        {
            level=DsHandleError();          /* Call user defined errorhandler */
            if ( ISTYP(GLOB(erritem),TYPE_PRT)        /* or error in IO port? */
            && ( (CELPRT(GLOB(erritem))==GLOB(inport))    /* IO port is stdin */
                || (CELPRT(GLOB(erritem))==GLOB(outport))))      /* or stdout */
                level=2;                      /* Can't go on with these ports */
            if (level==1) level=0;         /* warnings and errors are handled */
	    GLOB(curenv)=curenv;
        GLOB(evaldepth)=0;
        }   
    }
    while (level==0)                 /* as long as no critical error occurred */
    {
        if (GLOB(verbose))
        {
            DsOuts(GLOB(outport),GLOB(prompt));
            DsFlushPort(GLOB(outport));
        }
        item=DsRead(GLOB(inport));                 /* read an item from stdin */
        if (item==Q_eof) DSVERROR(ERRXIT);                    /* end of input */
        item=DsEval(item);                                     /* evaluate it */
        curenv = GLOB(curenv);                   /* Lock onto new environment */
        if (GLOB(verbose))
            DsOutc(GLOB(outport),item,TRUE);
    }
#ifdef GCPORTS
    POP;                                               /* reset stack (ports) */
#endif
    memcpy(GLOB(err_jmp),oldjmp,sizeof(oldjmp));     /* restore old jmp adres */
    return level==2 ? S_ERROR : S_END;                  /* end of this waiter */
}



/***************************************************************
** NAME:        DsNewGlobal
** SYNOPSIS:    GLOBAL *DsNewGlobal;
** DESCRIPTION: Allocates a new block of global variables.
**              Calls DsError when out of memory.
** RETURNS:     pointer to new global variables block.
***************************************************************/
STATIC
GLOBAL * PASCAL DsNewGlobal()
{
    GLOBAL *gp;
#ifndef FIXEDGLO
    GETMEM(gp,GLOBAL,sizeof(GLOBAL),"Global Structure");
#else
    gp=&DsGlo;
#endif
    memset(gp,0,sizeof(GLOBAL));
    gp->verbose = TRUE;
    gp->magic   = COOKIE;
    return(gp);
}


/*****************************************************************
** NAME:        DsIniEStack
** SYNOPSIS:    void DsIniEStack(size);
**              int size;
** DESCRIPTION: Initializes the evaluator stack.
** RETURNS:     void
*****************************************************************/
STATIC
void PASCAL DsIniEStack(size)
int size;
{
    if (GLOB(sstack)!=NIL)
        FFREE(GLOB(sstack));
    GLOB(sstack)=(CELPP)FARMALLOC(size, CELP);
    if (ISNIL(GLOB(sstack)))
        DsMemError("S-stack");
    GLOB(stkptr)=GLOB(sstack);                           /* stackptr at begin */
    GLOB(estack)=GLOB(sstack)+size;                           /* end of stack */
}


/*****************************************************************
** NAME:        DsIniEnvironment
** SYNOPSIS:    void DsIniEnvironment();
** DESCRIPTION: Initializes the system & user environment space.
** RETURNS:     void
*****************************************************************/
STATIC
void PASCAL DsIniEnvironment()
{
    if (ISNIL(GLOB(sysenv)))
    {
        GLOB(sysenv) = DsCons1(DsCons1(NIL));
        GLOB(curenv) = GLOB(sysenv);
        GLOB(topexp) = NIL;
        GLOB(curexp) = NIL;
        DsIniConst();                            /* init constants after init */
        DsIniIO();
    }
}



/*****************************************************************
** NAME:        DsIniHash
** SYNOPSIS:    void DsIniHash(hashsize)
**              int hashsize;
** DESCRIPTION: Initializes the hashtable.
** RETURNS:     void
*****************************************************************/
STATIC
void PASCAL DsIniHash(hashsize)
int hashsize;
{
    int i;

    if (hashsize>0)
    {
        if (GLOB(hashtab)!=NULL)
            DSVERROR(ERRHASH);
        GETMEM(GLOB(hashtab),CELP,(1+hashsize)*sizeof(CELP),"Hash Table");
        GLOB(hashsize)=hashsize;
        for (i=0;i<hashsize;i++)
           GLOB(hashtab)[i]=NIL;
    }
}


/*****************************************************************
** NAME:        DsSetStdPorts
** SYNOPSIS:    void DsSetStdPorts(inport,outport,errport)
**              PORT *inport,*outport,*errport;
** DESCRIPTION: Initializes the standard IO ports.
**              If no other ports are given, and no ports are
**              opened, then the standard in, out and err-ports
**              are opened.
** RETURNS:     void
*****************************************************************/
STATIC
void PASCAL DsSetStdPorts(inport,outport,errport)
PORT *inport,*outport,*errport;
{
    if (inport!=NIL)
    {
        DsClosePort(GLOB(inport));
        GLOB(inport)=inport;                                /* store new port */
    }
    if (outport!=NIL)
    {
        DsClosePort(GLOB(outport));                        /* close old ports */
        GLOB(outport)=outport;                           /* install new ports */
    }
    if (errport!=NIL)
    {
        DsClosePort(GLOB(errport));                        /* close old ports */
        GLOB(errport)=errport;                           /* install new ports */
    }
    if (GLOB(inport)==NIL)
        GLOB(inport)=GLOB(sinport);
    if (GLOB(outport)==NIL)                /* no port defined => use standard */
        GLOB(outport)=GLOB(soutport);
    if (GLOB(errport)==NIL)                /* no port defined => use standard */
        GLOB(errport)=GLOB(serrport);
}


/***************************************************************
** NAME:        DsIniConst
** SYNOPSIS:    void DsIniConst(void);
** DESCRIPTION: Initializes all the kernel functions and links
**              them to the system environment.
** RETURNS:     void
***************************************************************/
STATIC
void PASCAL DsIniConst()
{
    int i;
    CELP newkey;

    GLOB(conssym)=FARMALLOC(IP_MAXCONST, CELP);
    if (ISNIL(GLOB(conssym)))
        DsMemError("Constant Symbol space");
    for (i=0;i<IP_MAXCONST;i++)
    {
        GLOB(conssym[i])=newkey=DsInsExt(funs[i].name);
        TAGpart(newkey)=TYPE_KEY;
        KEYpart(newkey)=i;
        ARGpart(newkey)=funs[i].nargs;
    }
    Q_true = CSYM(IP_TRUE);
    Q_invis = CSYM(IP_INVIS);
    TAGpart(Q_true) = TYPE_SPC;
    TAGpart(Q_invis)= TYPE_SPC;
    TAGpart(Q_undef)= TYPE_SPC;
    TAGpart(Q_else) = TYPE_SPC;
    TAGpart(Q_eof)  = TYPE_SPC;
}


/***************************************************************
** NAME:        DsCtrlHandler
** SYNOPSIS:    void DsCtrlHandler(sig)
**              int sig;
** DESCRIPTION: Called when Ctrl-C is pressed. This function
**              is not part of DScheme, but is used for testing
**              purposes.
** RETURNS:     void
***************************************************************/
STATIC
void CDECL DsCtrlHandler(sig)
int sig;
{
    int c;

    sig;                      /* disable compiler warning about unreferenced! */
    signal(SIGINT, SIG_IGN);
    DsOut(GLOB(errport),'\n');
    while (1)                              /* until a correct answer is given */
    {
        DsOutf(GLOB(errport),"*** B)reak evaluation, C)ontinue, Q)uit to %s or S)tatus? ",SYSTEEM);
        DsFlushPort(GLOB(errport));
#ifndef MSC
        c=getchar();    /* should be a get with no buffering */
    	while (getchar()!='\n');
#else
        c=getche();putchar('\n');
#endif
        c=toupper(c);
        signal(SIGINT, DsCtrlHandler);
        switch (c)
        {
        case 'C': return;
        case 'B': DSVERROR(ERRBREAK);break;
        case 'Q': DSVERROR(ERRXIT);break;
        case 'S': Ds_globals();break;
        default : putchar(BELL);                 /* Produce an audible alert! */
        }
    }
}



#ifdef AKA
/***************************************************************
** NAME:        strupr
** SYNOPSIS:    char *strupr(str)
**              char *str;     pointer to input string
** DESCRIPTION: strupr converts a string to uppercase.
** RETURNS:     Pointer to (converted) string.
***************************************************************/
char *strupr(p)
char *p;
{
    char *q;
    for (q=p;*q;q++) 
	 if (islower(*q)) *q += ('A'-'a');
    return(p);
}

/***************************************************************
** NAME:        stricmp
** SYNOPSIS:    int stricmp(s1,s2)
**              char *s1;     pointer to input string
**              char *s2;     pointer to input string
** DESCRIPTION: A case-insensitive version of strcmp.
** RETURNS:     <0: s1 is smaller than s2 (alphabeticly)
**              =0: s1 is equal to s2
**              >0: s1 is bigger than s2 (alphabeticly)
***************************************************************/
int stricmp(s1,s2)
char *s1;
char *s2;
{
    while (*s1)
    {
        register int d = tolower(*s1)-tolower(*s2);
	if (d) return d;
	s1++,s2++;
    }
    return (*s2)?-1:0;
}
#endif




