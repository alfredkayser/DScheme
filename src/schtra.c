/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHTRA.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/05/09
**
** DESCRIPTION: This module contains functions to
**              trace the DScheme interpreter.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schtra.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHTRA.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHTRA.C_V  $
**
**                 Rev 1.0   12 Oct 1989 11:45:36   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"

STATIC void PASCAL DsTraMessage __((int c, CELP item));
STATIC void PASCAL DsTraceDummy __((int position));
STATIC void PASCAL DsTraceHook  __((int position));
STATIC CELP CDECL  Ds_strace    __((CELP arg));
STATIC CELP CDECL  Ds_gtrace    __((void));

STATIC EXTDEF extensions[]=
    {
        {"GET-TRACE",  (EXTPROC)Ds_gtrace, 0},
        {"SET-TRACE!", (EXTPROC)Ds_strace, 1, TYPE_INT},
        ENDOFLIST
    };

#define LEVEL_OFF   0x00
#define LEVEL_EVAL  0x01
#define LEVEL_APPLY 0x02
#define LEVEL_TAIL  0x04
#define LEVEL_MACRO 0x08
#define LEVEL_SELF  0x10

TCALL tracer = DsTraceDummy;
int   trace = 0;

/***************************************************************
** NAME:        DsTraceInit
** SYNOPSIS:    int DsTraceInit()
** DESCRIPTION: Initializes and link some trace functions to the
**              DScheme system.
** RETURNS:     S_ERROR, if error occured.
**              S_OKAY otherwise.
***************************************************************/
void PASCAL DsTraceInit()
{
    trace = 0;
    tracer = DsTraceDummy;
    DsFuncDef(extensions);        /* link extended functions */
}


STATIC
CELP Ds_strace(arg)
CELP arg;
{
    trace = (int)CELINT(arg);
    tracer = (trace) ? (TCALL)DsTraceHook : (TCALL)DsTraceDummy;
    return(arg);
}


STATIC
CELP Ds_gtrace()
{
    return DSINTCEL(trace);
}

/***************************************************************
** NAME:        DsTraceDummy
** SYNOPSIS:    void PASCAL DsTraceDummy(n);
**              int n;
** DESCRIPTION: Dummy function.
** RETURNS:     void
***************************************************************/
STATIC
void PASCAL DsTraceDummy(place)
int place;
{
    place;
}

/***************************************************************
** NAME:        DsTraceHook
** SYNOPSIS:    void DsTraceHook(key,n);
**              CELP key;      The keyword to apply
**              int position;  Position in the evaluator
** DESCRIPTION: Prints a trace message, when the corresponding
**              trace bit is set.
**              Currently there are four trace bits defined:
**              1. Entering, tailing and returning complex evals
**              2. Apply and return.
**              4. Macro substitutions.
**              8. Self evaluating expressions
** RETURNS:     void
***************************************************************/
STATIC
void PASCAL DsTraceHook(position)
int position;
{
    switch(position)
    {
    case T_START:                  /* start complex evaluation */
        if (trace&LEVEL_EVAL)
        {
            DsTraMessage('E',item);
            GLOB(evaldepth)++;
        }
        break;

    case T_APPLY:
        if (trace&LEVEL_APPLY)
            DsTraMessage('A',DsCons(key,item));
        break;

    case T_END:                     /* end complex evaluation */
        if (trace&LEVEL_EVAL)
            if (GLOB(evaldepth)>0) GLOB(evaldepth)--;
        if (trace&(LEVEL_APPLY|LEVEL_EVAL))
            DsTraMessage('R',item);
        break;

    case T_TAIL:
        if (trace&LEVEL_TAIL) DsTraMessage('T',item);
        break;

    case T_MACRO:
        if (trace&LEVEL_MACRO) DsTraMessage('M',item);
        break;

    case T_SELF:             
        if (trace&LEVEL_SELF) DsTraMessage('S',item);
        break;

    case T_LOOKUP:
        if (trace&LEVEL_MACRO) DsTraMessage('L',item);
        break;
    }
}


/***************************************************************
** NAME:        DsTraMessage
** SYNOPSIS:    void DsTraMessage(c,item);
**              char c;         Trace message character.
**              CELP item;      The expression to trace.
** DESCRIPTION: Prints a trace message.
** RETURNS:     void
***************************************************************/
STATIC void PASCAL DsTraMessage(c,item)
int c;
CELP item;
{
#ifdef INDENT    
    int n;
    DsOutf(GLOB(errport),"TRACE %c:",c);
    for (n=GLOB(evaldepth);n;n--) DsOut(GLOB(errport),' ');
    DsOutf(GLOB(errport),"%a\n", item);
#else
    DsOutf(GLOB(errport),"TRACE %c: (%d) %a\n", (c), GLOB(evaldepth), (item));
#endif
}





