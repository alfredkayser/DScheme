/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHTST.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/02/29
**
** DESCRIPTION: Test main module for the SCHEME system.
**              Contains dummy "main".
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   ALFRED  $
** WORKFILE:    $Workfile:   schtst.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHTST.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHTST.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:45:14   ALFRED
**              Initial revision.
**********************************************************************/
#include <fcntl.h>
#ifndef UNIX
#include <io.h>
#endif
#include "schinc.h"         /* needed for DScheme.lib */

STATIC void PASCAL werror  __((CONST char *mes));
STATIC void PASCAL quit    __((int ret_code));

/***************************************************************
** NAME:        main
** SYNOPSIS:    void main(argc,argv)
**              int argc;
**              char **argv;
** DESCRIPTION: Initializes the SCHEME system for a test drive
** RETURNS:     Error code
** SEE ALSO:    Scheme
***************************************************************/
GLOBAL *job1=NIL;

void main(argc,argv)
int argc;
char **argv;
{
    LONG numcels;

    numcels=6550L; /*default*/
#ifndef MSDOS
    setvbuf(stdin,NULL,_IONBF,0);/*force immediate flushing of each character */
    setvbuf(stdout,NULL,_IONBF,0); /* stderr is already unbufferd */
#endif
    info();
    if (argc==2)
    {
        numcels=atol(argv[1]);
        if (numcels==0)
            werror("Error: first argument must be a number.\nUsage: 'DScheme <number>' or 'DScheme'\n");
    }
    if (DSinit(&job1, S_NUMCEL,    numcels,
                      S_STRING,    10000,
                      S_STACK,     8000,
                      S_HASHSIZE,  997,
                      S_PROMPT,    "\nDS=> ",
                      S_CTRLC,
                      0)==S_OKAY)
    {
        if (DSmath(job1)!=S_OKAY ||
/*          DSutinit(job1)!=S_OKAY ||     * Uncomment this if utlib is needed */
            DSextend(job1)!=S_OKAY)
            werror("Errors detected while initializing DScheme\n");
        DScheme(job1);
    }
    quit(0);
}


/***************************************************************
** NAME:        DSGCmessage
** SYNOPSIS:    void DSGCmessage(global,mesnr)
**              GLOBAL *global; Pointer to environment
**              int mesnr;      Message nummer
** DESCRIPTION: To be supplied by the user. This function is
**              called by the garbage collector to indicate its
**              progress. Mesnr can be: GCstart,GCrun and GSstop
**              These are defined in scheme.h
** RETURNS:     void
***************************************************************/
void PASCAL DSGCmessage(nr)
int nr;
{
#ifndef GCDEBUG
    static int smiley='\\';
    if (nr==GCstop)         /* end of GC? */
        putchar(' ');
    else
    {
        putchar(smiley);
        smiley ^= '\\'^'/';
    }
    putchar('\b');
#else
    static LONG lastfree;
    switch(nr)
    {
    case GCstart:
        lastfree=GLOB(freecels);
        DsOuts((GLOB(soutport),"\n*** Garbage Collecting ***           ");
        break;

    case GCrun:
        if (GLOB(freecels)>lastfree)
            DsOutf( (GLOB(soutport),
                    "\r*** GC: %ld cells released ***       ",
                    GLOB(freecels)-lastfree);
        break;

    case GCstop:
        if (GLOB(freecels)>lastfree)
            DsOutf( (GLOB(soutport),
                    "\r*** GC: %ld cells released ***       \n",
                    GLOB(freecels)-lastfree);
        else
            DsOuts( (GLOB(soutport),
                    "\r*** GC: No garbage found! ***        \n");
        break;
    }
#endif
}


STATIC void PASCAL werror(mes)
CONST char *mes;
{
    fprintf(stderr,"SCHTST: %s\n",mes);
    quit(1);
}


STATIC void PASCAL quit(ret_code)
int ret_code;
{
    if (job1)
    {
        DsOutf(job1->soutport,"Back to %s\n",SYSTEEM);
        DSclose(&job1);
    }
    exit(ret_code);
}

