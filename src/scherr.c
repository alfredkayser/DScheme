/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHERR.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/05/09
**
** DESCRIPTION: This module contains the error routines.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   scherr.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHERR.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHERR.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:45:30   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"

STATIC char *ErrMsg[] = {
    "End of DScheme",
    "Out of memory",
    "Internal error (contact A.Kayser at TUDelft)",
    "Exit",
    "NIL pointer",
    "Hash table allready initialized",
    "Internal: Unknown cell type",
    "EVAL: Not (yet) implemented",
    "EVAL: Wrong argument count",
    "EVAL: Wrong argument type",
    "EVAL: CAR needs a pair as argument",
    "EVAL: CDR needs a pair as argument",
    "EVAL: This is not a self-evaluating atom",
    "EVAL: Can't evaluate this procedure",
    "EVAL: Unknown symbol",
    "EVAL: Evaluation stack overflow",
    "Internal use: Break",
    "IO: Output error (disk or string full)",
    "IO: Read error (port is closed or write-only)",
    "IO: Write error (port is closed or read-only)",
    "IO: Out of port space",
    "IO: port is closed!",
    "IO: Port table allready initialized",
    "IO: Port table is full",
    "IO: File not found",
    "IO: Unput buffer overflow",
    "Parser: DOT encountered in wrong place",
    "Parser: Syntax error",
    "Parser: Premature EOF",
    "Parser: Too many ')'s",
    "Parser: Real numbers don't support binair, octal and hex numbers",
    "Parser: Number expected after +/-",
    "Parser: Quote argument missing",
    "Parser: String too long",
    "Parser: Internal error",
    "Parser: Illegal character encountered",
    "Parser: Illegal character(s) after '#'",
    "Parser: Illegal character in number",
    "Parser: Illegal floating point format",
    "Parser: Only integers may follow '#!'",
    "Timestamp: out of range",
    "Timestamp: syntax error",
    "Timestamp: error in year",
    "Timestamp: error in month",
    "Timestamp: error in day",
    "Timestamp: error in hours",
    "Timestamp: error in minutes",
    "Timestamp: error in seconds",
    "Math: bignumber too big",
    "Math: divide by zero",
    "Math: floating point overflow",
    "Math: bignumber math not present",
    "Math: Floating point error",
    "Parser: 3 octal digits expected in string escape"
    };


/***************************************************************
** NAME:        DsNoInitError
** SYNOPSIS:    void DsNoInitError(function)
**              char *function;     function name
** DESCRIPTION: called when a DS function is called with the
**              global structure pointing to NIL.
***************************************************************/
void PASCAL DsNoInitError(function)
CONST char *function;
{
    fprintf(stderr,
            "Function %s is called before initialization of the environment\n",
            function);
    exit(2);
}


/***************************************************************
** NAME:        DsRetCode
** SYNOPSIS:    int DsRetCode();
** DESCRIPTION: Called by DS??? functions when an error is
**              raised, to clean up the environment and to
**              determine what return code should be given to
**              the application.
** RETURN:      S_ERROR, if really an error occured.
**              S_END, if the DScheme 'exit' is evaluated.
**              S_OKAY, otherwise.
***************************************************************/
int PASCAL DsRetCode()
{
    switch(GLOB(errnr))
    {
    case NOERROR: return(S_OKAY);                            /* normal return */
    case ERRXIT:  return(S_END);         /* DScheme has evaluated an "(exit)" */
    default:      DsHandleError();           /* handle error, display message */
                  return(S_ERROR);                  /* got an (serious) error */
    }
}


/***************************************************************
** NAME:        DsHandleError
** SYNOPSIS:    void DsHandleError()
** DESCRIPTION: This function is called by the reader to display
**              a neat error message.
** RETURNS:     void
***************************************************************/
int PASCAL DsHandleError()
{
    int val=GLOB(errfunc)(PGLOBAL,GLOB(errnr),GLOB(erritem));
    GLOB(errnr)=NOERROR;
    return val;
}


/***************************************************************
** NAME:        DsMemError
** SYNOPSIS:    void DsMemError(mes);
** DESCRIPTION: Signals a memory error to stderr. Called by
**              initialization functions.
** RETURN:      void
***************************************************************/
void PASCAL DsMemError(mes)
CONST char *mes;
{
    fprintf(stderr,"DScheme: INIT: no memory for %s\n",mes);
    exit(1);
}


/***************************************************************
** NAME:        DsTypError
** SYNOPSIS:    void DsTypError(q)
**              CELP q;         Argument in error
** DESCRIPTION: Signals a argument type error. Shorthand for
**              DsError(ERRART,q);
** RETURN:      void
***************************************************************/
void PASCAL DsTypError(q)
CELP q;
{
    DsError(ERRART,q);
}


/***************************************************************
** NAME:        DsStkError
** SYNOPSIS:    void DsStkError()
** DESCRIPTION: Signals a stack error. Shorthand for
**              DsError(ERRSTK,Q_invis);
** RETURN:      void
***************************************************************/
void PASCAL DsStkError()
{
    DsVError(ERRSTK);
}

/***************************************************************
** NAME:        DsVError
** SYNOPSIS:    void DsVError(nr)
**              int nr;
** DESCRIPTION: Raises error <nr> with a void argument.
**              same as DsError(nr,Q_invis);
** RETURN:      void
***************************************************************/
void PASCAL DsVError(nr)
int nr;
{
    DsError(nr,Q_invis);
}

/***************************************************************
** NAME:        DsError
** SYNOPSIS:    CELP DsError(nr,item);
**              int     nr;       Error number
**              CELP    item;     Item
** DESCRIPTION: Stores error number in glo->errnr and jmps to
**              glo->err_jmp. (the 'main' function).
** RETURNS:     Doesn't return to caller!
***************************************************************/
void PASCAL DsError(err_nr,item)
int err_nr;
CELP item;
{
    if (err_nr==ERRMEM) GLOB(GCflag)=3; /* Garbage Collect a.s.a.p */
    GLOB(errexp)=GLOB(curexp);
    GLOB(erritem)=item;
    GLOB(errnr)=err_nr;
    /* GLOB(errline)=GLOB(inport)->lineno; */
    longjmp(GLOB(err_jmp),1);      /* severe error */
}                               


/***************************************************************
** NAME:        DsPError
** SYNOPSIS:    void DsPError(nr,item);
**              int     nr;       Error number
**              PORT    *item;    Port in error
** DESCRIPTION: Stores error number in glo->errnr and jmps to
**              glo->err_jmp.
**              Same as DsError, but this one is for port errors
** RETURNS:     Doesn't return to caller!
***************************************************************/
void PASCAL DsPError(err_nr,item)
int err_nr;
PORT *item;
{
    DsError(err_nr,DSPRTCEL(item));
}


/**************************************************************
** NAME:        DsRepError
** SYNOPSIS:    int DsRepError(glo,errnr,item)
**              GLOBAL *glo;
**              int errnr;
** DESCRIPTION: Default error reporter.
** RETURNS:     error level.
**************************************************************/
int DsRepError(glo,errnr,item)
GLOBAL *glo;
int errnr;
CELP item;
{
    char *errlev;
    char *errmsg;

    if (errnr==ERRBREAK)
    {
        DsOuts(glo->errport,"*** BREAK ***\n");
        return(0);
    }
    if (errnr>0)
    {
        switch(DsErrorLevel(errnr))
        {
        case 0 : errlev="** ";   errmsg="Warning";      break;
        case 1 : errlev="*** ";  errmsg="ERROR";        break;
        default: errlev="**** "; errmsg="SEVERE ERROR"; break;
        }
        if (errnr<ERRUSER1)
        {
            DsOutf(glo->errport,"%s%s [%d] at line %l: %s\n",
                errlev, errmsg, errnr, GLOB(errline), DsErrMsg(errnr));
            if (item!=Q_invis)
                DsOutf(glo->errport,"%sItem in error: %a\n",errlev, item);
            if (glo->errexp)
                DsOutf(glo->errport,"%sExpression: %a\n",errlev, glo->errexp);
        }
        else
            DsOutf(glo->errport,"%s%s [%d] at line %l: %p\n",
                errlev, errmsg, errnr, GLOB(errline), item);
    }
    return(DsErrorLevel(errnr)); /* return error level */
}


/***************************************************************
** NAME:        DsErrMsg
** SYNOPSIS:    char *DsErrMsg(errnr)
**              int errnr;         Error nummer
** DESCRIPTION: Translates an error number in an error
**              description message.
** RETURNS:     The error description.
***************************************************************/
STATIC char * PASCAL DsErrMsg(errnr)
int errnr;
{
    return (errnr<0 || errnr>sizeof(ErrMsg)/sizeof(char *))
           ? "Unknown error: core dumped"
           : ErrMsg[errnr];
}


/***************************************************************
** NAME:        DsErrorLevel
** SYNOPSIS:    int DsErrorLevel(errnr)
**              int errnr;      error number.
** DESCRIPTION: determines the level of the error.
** RETURNS:     0=warning: continue evaluation.
**              1=error:   stop evaluation.
**              2=serious: stop WaiterLoop
***************************************************************/
STATIC int PASCAL DsErrorLevel(errornr)
int errornr;
{
    switch(errornr)
    {
    case NOERROR:                 /* not serious, but must get out of Dscheme */
    case ERRMEM:
    case ERRXIT:
    case ERRINT:
    case ERROUT:
    case ERRIOW:
    case ERRUSER2:
    case ERREOF:   return 2;                       /* close caf‚ after an EOF */
    case ERRUSER1: return 0;                                 /* warning level */
    }
    return 1;
}


void PASCAL DsStrError(typ,str)
int typ;
CONST char *str;
{
    DsError(typ,DsStrCell(str));
}



