/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHIO.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/17
**
** DESCRIPTION: The IO routines used by the SCHEME system
**              are defined in this file.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schio.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHIO.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHIO.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:46:26   JAN
**              Initial revision.
**********************************************************************/
#ifdef OS2
#define INCL_DOS
#include <os2.h>
#endif
#include "schinc.h"

#ifdef SUN
int fgetc __((UNTYPE *));
void fputc __((int, UNTYPE *));
#endif

STATIC CELP   PASCAL DsOFile    __((CELP str,int mode));
STATIC void   PASCAL DsOutg     __((PORT *outport, REAL g));
STATIC void   PASCAL DsOutx     __((PORT *outport, LONG x));
STATIC void   PASCAL DsOutl     __((PORT *outp, CELP elem, int mode));
STATIC void   PASCAL DsOutv     __((PORT *outp, CELP elem, int mode));
STATIC char * PASCAL DsString   __((CELP elem));
STATIC char * PASCAL DsChar     __((int ch));
STATIC void   PASCAL DsPSError  __((int num, UNTYPE *stream));
STATIC char * PASCAL DsItoa     __((long num));
STATIC PORT * PASCAL GetOPort   __((int n, CELP *pp));
#ifdef OS2
STATIC PORT * PASCAL  DsDosOpen     __((HFILE, USHORT));
#endif

/*-------------------- Scheme Procedures ------------------*/
STATIC CELP   CDECL  Ds_write          __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_writech        __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_readitem       __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_readch         __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_display        __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_newline        __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_flush          __((int nvals,CELP args));
STATIC CELP   CDECL  Ds_close          __((CELP args));
STATIC CELP   CDECL  Ds_oifile         __((CELP args));
STATIC CELP   CDECL  Ds_oofile         __((CELP args));
STATIC CELP   CDECL  Ds_getcip         __((void));
STATIC CELP   CDECL  Ds_getcop         __((void));
STATIC CELP   CDECL  Ds_getcep         __((void));

/*--------------------- Port Functions --------------------*/
STATIC int    PASCAL DsFInput   __((UNTYPE *stream));
STATIC void   PASCAL DsFOutput  __((int c, UNTYPE *stream));
STATIC void   PASCAL DsFControl __((UNTYPE *stream, int action));
STATIC CELP   CDECL  Ds_inport  __((CELP port));
STATIC CELP   CDECL  Ds_outport __((CELP port));
STATIC CELP   CDECL  Ds_errport __((CELP port));

static EXTDEF DsPortFuns[] =
    {
        {"WRITE",              (EXTPROC)Ds_write,    -1},
        {"WRITE-CHAR",         (EXTPROC)Ds_writech,  -1},
        {"DISPLAY",            (EXTPROC)Ds_display,  -1},
        {"READ",               (EXTPROC)Ds_readitem, -1},
        {"READ-CHAR",          (EXTPROC)Ds_readch,   -1},
        {"NEWLINE",            (EXTPROC)Ds_newline,  -1},
        {"FLUSH",              (EXTPROC)Ds_flush,    -1},
        {"GET-CIP",            (EXTPROC)Ds_getcip,    0},
        {"CURRENT-INPUT-PORT", (EXTPROC)Ds_getcip,    0},
        {"GET-COP",            (EXTPROC)Ds_getcop,    0},
        {"CURRENT-OUTPUT-PORT",(EXTPROC)Ds_getcop,    0},
        {"GET-CEP",            (EXTPROC)Ds_getcep,    0},
        {"CURRENT-ERROR-PORT", (EXTPROC)Ds_getcep,    0},
        {"CLOSE-PORT",         (EXTPROC)Ds_close,     1, TYPE_PRT},
        {"OPEN-INPUT-FILE",    (EXTPROC)Ds_oifile,    1, TYPE_STR},
        {"OPEN-OUTPUT-FILE",   (EXTPROC)Ds_oofile,    1, TYPE_STR},
        {"LOAD",               (EXTPROC)Ds_load,      1, TYPE_STR},
        {"SET-INPUT-PORT",     (EXTPROC)Ds_inport,    1, TYPE_PRT},
        {"SET-OUTPUT-PORT",    (EXTPROC)Ds_outport,   1, TYPE_PRT},
        {"SET-ERROR-PORT",     (EXTPROC)Ds_errport,   1, TYPE_PRT},
        ENDOFLIST
    };

extern CELP item;

/**************************************************************
** NAME:        DsIniIO
** SYNOPSIS:    void DsIniIO(void);
** DESCRIPTION: Initializes the Scheme IO procedures and
**              constants.
** RETURNS:     void
**************************************************************/
void PASCAL DsIniIO()
{
    PRTCEL(item,GLOB(sinport));
    DsDefVar(DsInsExt("STDIN"));
    PRTCEL(item,GLOB(soutport));
    DsDefVar(DsInsExt("STDOUT"));
    PRTCEL(item,GLOB(serrport));
    DsDefVar(DsInsExt("STDERR"));
    DsFuncDef(DsPortFuns); /* define port functions */
}


/*****************************************************************
** NAME:        DsIniPorts
** SYNOPSIS:    void DsIniPorts()
**              int numport;    Size of port table
**              int bigbuf;     Size of big string buffer
**              int numbuf;     Number of io buffers.
**              int bufsize;    Size of io buffer.
** DESCRIPTION: Initializes the port system & file io buffers.
** RETURNS:     void
*****************************************************************/
void PASCAL DsIniPorts(numport,bigbuf,numbuf,bufsize)
int numport,bigbuf,numbuf,bufsize;
{
    int i;

    if (numport>0)
    {
        if (GLOB(ports)!=NIL)
            DSVERROR(ERRPORT);
        GETMEM(GLOB(ports),PORT,numport*sizeof(PORT),"Port Table");
        for (i=0;i<numport;i++)
            GLOB(ports[i]).dir=FREE;
        GLOB(prtnum)=numport;

        if (bigbuf<MAXSTR) bigbuf=MAXSTR;     /* Safety Treshold */
        BIGMAX=bigbuf;
        GETMEM(BIGBUF, char, bigbuf,"BIGbuffer");
    }
}


/***************************************************************
** NAME:        DsInput
** SYNOPSIS:    int DsInput(inport);
**              PORT *inport;   The input port.
** DESCRIPTION: DsInput tries to read a character from a scheme
**              port. This can be from a file or a string. In
**              both cases the unput buffer is emptied first.
** RETURNS:     Character read from the port.
**              EOF when at end of file/string.
** SEE ALSO:    Sunput()
***************************************************************/
int PASCAL DsInput(inport)
PORT *inport;
{
    if (inport->dir & UNPUTTED)
        inport->dir ^= UNPUTTED;                     /* reset unputted flag */
    else
        inport->unput=inport->portin(inport->stream);
    if (inport->unput=='\n') inport->lineno++;
    return inport->unput;
}


/***************************************************************
** NAME:        DsUnput
** SYNOPSIS:    void DsUnput(inport,c);
**              PORT *inport;   The input port.
**              int c;         The character which is to be
**                              put back in the port.
** DESCRIPTION: DsUnput pushes the last read character back into
**              the port. (Only one char can be put back).
** RETURNS:     void.
** SEE ALSO:    DsInput()
***************************************************************/
void PASCAL DsUnput(inport,c)
PORT *inport;
int c;
{
    if (inport->dir & UNPUTTED) DSPERROR(ERRIOU,inport);
    if (c=='\n') inport->lineno--;
    inport->dir |= UNPUTTED;
    inport->unput = c;
}


/***************************************************************
** NAME:        DsPeek
** SYNOPSIS:    int DsPeek(inport);
**              PORT *inport;   The input port.
** DESCRIPTION: DsPeek reads a character from the input port
**              pushes it immediately back.
** RETURNS:     void.
** SEE ALSO:    DsInput()
***************************************************************/
int PASCAL DsPeek(inport)
PORT *inport;
{
    if (!(inport->dir & UNPUTTED))
    {
        inport->unput=inport->portin(inport->stream);     /* read a character */
        inport->dir |= UNPUTTED;
    }
    return inport->unput;
}


/***************************************************************
** NAME:        DsOut
** SYNOPSIS:    void DsOut(outport,c);
**              PORT *outport;  The output port.
**              int c;         The character which is to be
**                              put in the output port.
** DESCRIPTION: Puts the character c into the output port.
** RETURNS:     void.
** SEE ALSO:    DsInput()
***************************************************************/
#ifdef OLD
void PASCAL DsOut(outport,c)
PORT *outport;
int c;
{
    outport->portout(c,outport->stream);
}
#endif


/***************************************************************
** NAME:        DsOuts
** SYNOPSIS:    void DsOuts(outport,str);
**              PORT *outport;  The output port.
**              char *str;      The string of characters to be
**                              put in the output port.
** DESCRIPTION: Puts the string str into the output port.
** RETURNS:     void.
** SEE ALSO:    DsOut()
***************************************************************/
void PASCAL DsOuts(outport,str)
PORT *outport;
CONST char *str;
{
    register OUT_FUN portout=outport->portout;
    register UNTYPE *stream=outport->stream;
    while (*str)
        portout(*str++,stream);
}


/***************************************************************
** NAME:        DsOutf
** SYNOPSIS:    void DsOutf(outport,fmt,...);
**              PORT *outport;  The output port.
**              char *fmt;      The format string.
** DESCRIPTION: Puts the string str into the output port.
**              Possible format commando's:
**              %c: insert char.
**              %s: insert string.
**              %d: insert integer.
**              %l: insert long integer. 
**              %f: insert float.
**              %t: insert timestamp (time_t)
**              %a: insert cell.    (write mode)
**              %A: insert cell.    (display mode)
**              %x: insert hex integer.
** RETURNS:     void.
** SEE ALSO:    printf()
***************************************************************/
void DsOutf(outport, fmt VAR_ARGS)
PORT *outport;
CONST char *fmt;
VAR_DCL
{
    CONST char *p;
    va_list va;
    VAR_START(va,fmt);
    for (p=fmt;*p;p++)
    {
        if (*p=='%')
        {
            switch(*++p)
            {
            case 'a': DsOutc(outport,va_arg(va, CELP),TRUE);        break;
            case 'A': DsOutc(outport,va_arg(va, CELP),FALSE);       break;
            case 'c': DsOut (outport,va_arg(va, int));              break;
            case 'd': DsOuts(outport,DsItoa((LONG)va_arg(va,int))); break;
            case 'f': DsOutg(outport,va_arg(va, REAL));             break;
            case 'l': DsOuts(outport,DsItoa(va_arg(va, LONG)));     break;
            case 's': DsOuts(outport,va_arg(va, char *));           break;
            case 't': DsOuts(outport,CELTMS(DsMakeTime(va_arg(va,time_t))));break;
            case 'x': DsOutx(outport,va_arg(va, LONG));             break;
            default : DsOut (outport,(int)*p);
            }
        }
        else
            DsOut(outport,(int)*p);
    }
    va_end(va);
}


/***************************************************************
** NAME:        DsOutg
** SYNOPSIS:    void DsOutg(outport,g);
**              PORT *outport;  The output port.
**              REAL g;         The floating point number.
** DESCRIPTION: Puts the real g into the output port.
** RETURNS:     void.
***************************************************************/
STATIC
void PASCAL DsOutg(outport,g)
REAL g;
PORT *outport;
{
    static char buf[30];
    sprintf(buf,"%#g",g);
    DsOuts(outport,buf);
}


/***************************************************************
** NAME:        DsOutx
** SYNOPSIS:    void DsOutx(outport,x);
**              PORT *outport;  The output port.
**              LONG x;
** DESCRIPTION: Puts the hex x into the output port.
** RETURNS:     void.
***************************************************************/
STATIC
void PASCAL DsOutx(outport,x)
PORT *outport;
LONG x;
{
    static char buf[10];
    sprintf(buf,"%08x",x);
    DsOuts(outport,buf);
}


/***************************************************************
** NAME:        DsOutc
** SYNOPSIS:    void DsOutc(outp,elem,mode);
**              PORT *outp      The output port
**              CELP elem       The SCHEME item to be printed
**              int  mode       TRUE => output is SCHEME
** DESCRIPTION: Prints a SCHEME item. Item can be an atom or a
**              list. If item is a list then DsOutc calls
**              DsOutl which calls recursively DsOutc. This
**              goes wrong if elem points to a circular list
** RETURNS:     void
***************************************************************/
void PASCAL DsOutc(outp,elem,mode)
CELP elem;
PORT *outp;
int mode;
{
    if (ISNIL(elem))
    {
        DsOut(outp,'(');
        DsOut(outp,')');
        return;
    }
    switch (TAGpart(elem))
    {
    case TYPE_STR:
        DsOuts(outp,(mode)?DsString(elem):STRPpart(elem));
        break;

    case TYPE_TMS:
        DsOuts(outp,CELTMS(elem));
        break;

    case TYPE_CHR:
        if (mode)
            DsOuts(outp,DsChar(CHRpart(elem)));
        else
            DsOut(outp,CHRpart(elem));
        break;

    case TYPE_PRT:
        DsOuts(outp,"#PORT");
        break;

    case TYPE_VEC:
        DsOutv(outp,elem,mode);
        break;

    case TYPE_OID:
        DsOutf(outp,"#!%d",CELINT(elem));
        break;

    case TYPE_INT:
        DsOuts(outp,DsItoa(CELINT(elem)));
        break;

    case TYPE_FLT:
        DsOutg(outp,CELFLT(elem));
        break;

    case TYPE_PAIR:
        DsOutl(outp,elem,mode);
        break;

    case TYPE_PRC:
        DsOuts(outp,"#LAMBDA");
        break;

    case TYPE_SYM:
    case TYPE_EXT:
    case TYPE_KEY:
    case TYPE_SPC:
    case TYPE_SYMD:
        DsOuts(outp,STRPpart(CDRpart(elem)));
        break;

    case TYPE_FUN:
        DsOutf(outp,"[%a, %d, %d]",CDRpart(elem),KEYpart(elem),ARGpart(elem));
        break;

    case TYPE_MAC:
        DsOuts(outp,"#MACRO");
        break;

    case TYPE_FREE:
        DsOuts(outp,"#FREECEL");
        break;

    case TYPE_BIGN:
    case TYPE_BIGP:
        DsOuts(outp,DsBigStr(elem));
        break;

    }
}


/***************************************************************
** NAME:        DsOutl
** SYNOPSIS:    void DsOutl(outp,elem);
**              PORT *outp      The output port
**              CELP elem       The SCHEME item to be printed
** DESCRIPTION: Prints a SCHEME list. (Elem must be a list)
**              Each member of the list is printed via 'DsOutc'.
**              The list doesn't have to be a simple list. This
**              function checks also for dotted lists.
** RETURNS:     void
** SEE ALSO:    DsOutc, DsOut
***************************************************************/
STATIC
void PASCAL DsOutl(outp,elem,mode)
PORT *outp;
CELP elem;
int mode;
{
    char pre='(';                    /* pre is character to print before item */

    for (;ISPAIR(elem);elem=CDRpart(elem))   /* first item begins with a '(', */
    {                                  /* all others are separated with a ' ' */
         DsOut(outp,pre);
         DsOutc(outp,CARpart(elem),mode);
         pre=' ';
    }                                   /* p is no longer a pointer to a pair */
                               /* is elem is a simple list, then p is now NIL */
    if (ISTRUE(elem))                        /* if p is not nil a dot follows */
    {
         DsOuts(outp," . ");
         DsOutc(outp,elem,mode);
    }
    DsOut(outp,')');
}


/***************************************************************
** NAME:        DsOutv
** SYNOPSIS:    void DsOutv(outp,elem);
**              PORT *outp      The output port
**              CELP elem       The SCHEME item to be printed
** DESCRIPTION: Prints a SCHEME vector.
**              Each member of the vector is printed via DsOutc.
** RETURNS:     void
** SEE ALSO:    DsOutc, DsOut
***************************************************************/
STATIC
void PASCAL DsOutv(outp,elem,mode)
PORT *outp;
CELP elem;
int mode;
{
    char pre='(';                 /* pre is character to print before item */
    int i;

    DsOut(outp,'#');
    for (i=0;i<VECLpart(elem);i++)
    {
        DsOut(outp,pre);
        DsOutc(outp,VECPpart(elem)[i],mode);
        pre=' ';
    }
    DsOut(outp,')');
}


/***************************************************************
** NAME:        DsString
** SYNOPSIS:    void DsString(elem);
**              CELP elem       The SCHEME str to be printed
** DESCRIPTION: Prints a SCHEME character.
** RETURNS:     void
** SEE ALSO:    Printc, DsOut
***************************************************************/
STATIC
char * PASCAL DsString(elem)
CELP elem;
{
    char *p;
    char *q;

    q=BIGBUF;
    *q++='\"';
    for (p=STRPpart(elem);*p;p++)
    {
        switch(*p)
        {
        case BELL : *q++='\\';*q++='a';break;
        case '\b' : *q++='\\';*q++='b';break;
        case '\t' : *q++='\\';*q++='t';break;
        case '\n' : *q++='\\';*q++='n';break;
        case '\f' : *q++='\\';*q++='f';break;
        case '\r' : *q++='\\';*q++='r';break;
        case '\27': *q++='\\';*q++='e';break; /* escape */
        default   : if (isprint(*p))
			*q++=*p; 
                    else
		    {
			*q++='\\';
			*q++='0'+(((*p)>>6)&7);
			*q++='0'+(((*p)>>3)&7);
			*q++='0'+(((*p)>>0)&7);
		    }
        }
    }
    *q++='\"';*q='\0';
    return BIGBUF;
}


/***************************************************************
** NAME:        DsChar
** SYNOPSIS:    char *DsChar(elem);
**              int elem       The SCHEME char to be printed
** DESCRIPTION: Prints a SCHEME character.
** RETURNS:     void
** SEE ALSO:    Printc, DsOut
***************************************************************/
STATIC
char *PASCAL DsChar(ch)
int ch;
{
    static char normal[4]="#\\a";
    switch(ch)
    {
    case BELL : return "#\\BELL";
    case '\b' : return "#\\BACKSPACE";
    case '\t' : return "#\\TAB";
    case '\n' : return "#\\NEWLINE";
    case '\f' : return "#\\FORMFEED";
    case '\r' : return "#\\RETURN";
    case '\27': return "#\\ESCAPE";
    case ' '  : return "#\\SPACE";
    }
    normal[2]=(char)ch;
    return normal;
}
                

/***************************************************************
** NAME:        DsInitPort
** SYNOPSIS:    PORT *DsInitPort(mode, stream, inputp,
**                             outputp, controlp)
**              int mode;
**              UNTYPE *stream;
**              INP_FUN inputp;
**              OUT_FUN outputp;
**              CTL_FUN controlp;
** DESCRIPTION: Request for a IO port.
**              Finds an empty port slot in the global port
**              resources and initializes it.
** RETURNS:     Pointer to the allocated and initialized port.
***************************************************************/
PORT * PASCAL DsInitPort(mode,stream,inputp,outputp,closep)
int mode;
UNTYPE *stream;
INP_FUN inputp;
OUT_FUN outputp;
CTL_FUN closep;
{
    int i;
    PORT *tmp;

    tmp=GLOB(ports);
    for (i=0;i<GLOB(prtnum);i++,tmp++)
        if (tmp->dir==FREE) break;                          /* find free port */
    if (i==GLOB(prtnum))                                /* no free port found */
        DSVERROR(ERRPORTF);

    tmp->dir     = mode;
    tmp->lineno  = 1L;
    tmp->stream  = stream;
    tmp->portin  = inputp;
    tmp->portout = outputp;
    tmp->portctl = closep;
    return(tmp);
}


/***************************************************************
** NAME:        DsClosePort
** SYNOPSIS:    void DsClosePort(port);
**              PORT *port;
** DESCRIPTION: Closes the SCHEME port.
**              Structure port is also released to port space.
** RETURNS:     void
***************************************************************/
void PASCAL DsClosePort(prt)
PORT *prt;
{
    if ((prt->dir&READWRIT) && !(prt->dir&STANDARD))/* is port really opened? */
    {
        prt->dir=FREE;                                  /* release for re-use */
        prt->portctl(prt->stream,IOCLOSE);                 /* close IO stream */
    }
}


/***************************************************************
** NAME:        DsFlushPort
** SYNOPSIS:    void DsFlushPort(port);
**              PORT *port;
** DESCRIPTION: Flushes the SCHEME port.
** RETURNS:     void
***************************************************************/
void PASCAL DsFlushPort(prt)
PORT *prt;
{
    if (prt->dir&WRITMODE)
        prt->portctl(prt->stream,IOFLUSH);                 /* close IO stream */
}
                
/*---------------------------FILE IO (standard)----------------------------*/


/***************************************************************
** NAME:        DsFOpen
** SYNOPSIS:    PORT *DsFOpen(name,mode);
**              char *name;     name of file to be opened.
**              int   mode;     filemode
** DESCRIPTION: Opens a SCHEME port (standard file port)
**              mode can be READMODE, WRITMODE or READWRIT.
** RETURNS:     Return pointer to port.
**              If port can't be opened, NIL is returned.
***************************************************************/
PORT *PASCAL DsFOpen(fname,mode)
CONST char *fname;
int   mode;
{
    FILE *fp;
    PORT *pp;

    switch (mode)
    {
    case READMODE: fp=fopen( fname, "r"); break;
    case WRITMODE: fp=fopen( fname, "w"); break;
    case READWRIT: fp=fopen( fname, "a"); break;
    default: DSVERROR(ERRINT);
    }
    if (ISNIL(fp)) return(NIL);
    switch(mode)
    {
    case READMODE:
        pp = DsInitPort( READMODE, (UNTYPE *)fp,
                         DsFInput, DsEOutput, DsFControl);
        break;
    case WRITMODE:
        pp = DsInitPort( WRITMODE, (UNTYPE *)fp,
                         DsEInput, DsFOutput, DsFControl);
        break;
    default:
        pp = DsInitPort( READWRIT, (UNTYPE *)fp,
                         DsFInput, DsFOutput, DsFControl);
        break;
    }
    return(pp);
}




/***************************************************************
** NAME:        DsEInput
** SYNOPSIS:    char DsEInput(stream)
**              UNTYPE *stream;   Pointer to file stream.
** DESCRIPTION: Called by DsInput when a file port is encountered
**              and port mode is WRITMODE.
** RETURNS:     never
***************************************************************/
int PASCAL DsEInput(stream)
UNTYPE *stream;
{
    STOP(DsPSError(ERRIOR,stream));
    return 0;
}


/***************************************************************
** NAME:        DsEOutput
** SYNOPSIS:    void DsEOutput(stream,c)
**              UNTYPE *stream;   Pointer to file stream.
** DESCRIPTION: Called by DsOutut when a file port is
**              encountered, and mode is READMODE.
** RETURNS:     void
***************************************************************/
void PASCAL DsEOutput(c,stream)
int c;
UNTYPE *stream;
{
    c;
    STOP(DsPSError(ERRIOW,stream));
}


/***************************************************************
** NAME:        DsFInput
** SYNOPSIS:    int DsFInput(stream)
**              UNTYPE *stream;   Pointer to file stream.
** DESCRIPTION: Reads one character from a file stream.
** RETURNS:     character read (-1 is EOF)
***************************************************************/
STATIC int PASCAL DsFInput(stream)
UNTYPE *stream;
{
    return getc((FILE *)stream);
}


/***************************************************************
** NAME:        DsFOutput
** SYNOPSIS:    void DsFOutput(stream,c)
**              UNTYPE *stream;   Pointer to file stream.
** DESCRIPTION: Reads one character from a file stream.
** RETURNS:     void
***************************************************************/
STATIC void PASCAL DsFOutput(c,stream)
int c;
UNTYPE *stream;
{
    putc(c,(FILE *)stream);
}


/***************************************************************
** NAME:        DsFControl
** SYNOPSIS:    char DsFControl(stream,nr)
**              UNTYPE *stream;   Pointer to file stream.
**              int action;       action to perform.
** DESCRIPTION: Called by DsControlPort when a file port is to be
**              controlled. The second argument is defined by the
**              DsInitPort call, and is used for file IO as
**              IObuffer nummer.
** RETURNS:     void.
***************************************************************/
STATIC void PASCAL DsFControl(stream,action)
UNTYPE *stream;
int action;
{
    switch(action)
    {
    case IOCLOSE:
        fclose((FILE *)stream);
        break;

    case IOFLUSH:
        break;
    }
}


/**************************************************************
** NAME:        DsPSError
** SYNOPSIS:    void PASCAL DsPSError(num,stream)
**              int num;
**              UNTYPE *stream;
** DESCRIPTION: Finds the port cell with the stream and raises
**              an IO error.
** RETURNS:     never
**************************************************************/
STATIC void PASCAL DsPSError(num,stream)
int num;
UNTYPE *stream;
{
    int i;

    for (i=0;i<GLOB(prtnum);i++)
        if (GLOB(ports)[i].stream==stream)
            DSPERROR(num,GLOB(ports)+i);
    DSVERROR(num);
}


/***************************************************************
** NAME:        DsStdPorts
** SYNOPSIS:    void DsStdPorts();
** DESCRIPTION: Initializes the standard ports connected to
**              'stdin', 'stdout' and 'stderr'.
** RETURNS:     void
***************************************************************/
void PASCAL DsStdPorts()
{
#ifdef OS2
    GLOB(outport)  =
    GLOB(soutport) = DsDosOpen(1, WRITMODE | STANDARD);
    GLOB(errport)  =
    GLOB(serrport) = DsDosOpen(2, WRITMODE | STANDARD);
#else
    GLOB(outport) =
    GLOB(soutport) = DsInitPort( WRITMODE | STANDARD,
                                 (UNTYPE *)stdout,
                                 DsEInput,
                                 DsFOutput,
                                 DsFControl);
    GLOB(serrport) =
    GLOB(errport) = DsInitPort( WRITMODE | STANDARD,
                                (UNTYPE *)stderr,
                                DsEInput,
                                DsFOutput,
                                DsFControl);
#endif
    GLOB(inport) =
    GLOB(sinport) = DsInitPort( READMODE | STANDARD,
                                (UNTYPE *)stdin,
                                DsFInput,
                                DsEOutput,
                                DsFControl);
}


STATIC char * PASCAL DsItoa(num)
DWORD num;
{
#ifdef MSC
    static char buf[12];
    return ltoa(num,buf,10);
#else
    return ltostr(num,10);
#endif
}

char * PASCAL ltostr(num, base)
long num;
int base;
{
    static char digits[]="0123456789abcdefghijklmnopqrstuvwxyz";
    register char *q;
    int sign;

    q=&BIGBUF[BIGMAX];
    *--q='\0';
    sign=0;
    if (num<0) sign++,num=-num;
    while (num>=base)
    {
	*--q=digits[num%base];
	num/=base;
    }
    *--q=digits[num];
    if (sign) *--q='-';
    return q;
}

STATIC PORT * PASCAL GetOPort(n,pp)
int n;
CELP *pp;
{
    CELP q;
    switch(n)
    {
    case 1: return GLOB(outport);
    case 2: q=CARpart(*pp);
            TYPCHECK(q,TYPE_PRT);
            *pp=CDRpart(*pp);
            return CELPRT(q);
    default:DSVERROR(ERRARC);
    }
}


/*------------------------ DScheme Port Functions -----------------------*/
STATIC CELP Ds_write(nvals,args)
int nvals;                    
CELP args;                    
{
    PORT *prt=GetOPort(nvals,&args);
    DsOutc(prt,args,TRUE);  
    return(Q_invis);
}

STATIC CELP Ds_display(nvals,args)
int nvals;
CELP args;
{
    PORT *prt=GetOPort(nvals,&args);
    DsOutc(prt,args,FALSE);                        /* print the item */
    return(Q_invis);
}


/***************************************************************
** NAME:        Ds_writech
** SYNOPSIS:    CELP Ds_writech(nvals)
**              int nval;
**              CELP list;      List
** DESCRIPTION: Prints all the elements of the list
** RETURNS:     an unprintable cell.
***************************************************************/
STATIC CELP Ds_writech(n,p)
int n;
CELP p;
{
    PORT *prt=GetOPort(n,&p);
    TYPCHECK(p,TYPE_CHR);
    DsOut(prt,CELCHR(p));                              /* print the character */
    return Q_invis;
}


STATIC CELP Ds_newline(n,p)
int n;
CELP p;
{
    DsOut(GetOPort(n+1,&p),'\n');
    return Q_invis;
}


STATIC CELP Ds_flush(n,p)
int n;
CELP p;
{
    PORT *prt=GetOPort(n+1,&p); /* Fake a second argument */
    DsFlushPort(prt);
    return Q_invis;
}


/***************************************************************
** NAME:        Ds_readitem
** SYNOPSIS:    CELP Ds_readitem(n,largs)
**              int n;
**              CELP largs;
** DESCRIPTION: Reads an item from the port.
** RETURNS:     The read item.
***************************************************************/
CELP Ds_readitem(n,p)
int n;
CELP p;
{
    switch(n)
    {
    case 0: return DsRead(GLOB(inport));
    case 1: TYPCHECK(p,TYPE_PRT);
            return DsRead(CELPRT(p));
    default: DSVERROR(ERRARC);
    }
}


/***************************************************************
** NAME:        Ds_readch
** SYNOPSIS:    CELP Ds_read(n,largs)
**              int n;
**              CELP largs;
** DESCRIPTION: Reads an character from the port.
** RETURNS:     The read item.
***************************************************************/
CELP Ds_readch(n,p)
int n;
CELP p;
{
    PORT *prt;
    switch(n)
    {
    case 0: prt=GLOB(inport); break;
    case 1: TYPCHECK(p,TYPE_PRT); prt=CELPRT(p); break;
    default: DSVERROR(ERRARC);
    }
    return DSCHRCEL(DsInput(prt));
}


STATIC CELP Ds_getcip()
{                             
    return DSPRTCEL(GLOB(inport));
}


STATIC CELP Ds_getcop()
{                             
    return DSPRTCEL(GLOB(outport));
}

STATIC CELP Ds_getcep()
{                             
    return DSPRTCEL(GLOB(errport));
}

STATIC CELP Ds_close(arg)
CELP arg;
{
    DsClosePort(CELPRT(arg));
    return Q_invis;
}

STATIC CELP Ds_oifile(arg)
CELP arg;
{
    return DsOFile(arg,READMODE);
}

STATIC CELP Ds_oofile(arg)
CELP arg;
{
    return DsOFile(arg,WRITMODE);
}


STATIC
CELP PASCAL DsOFile(str,mode)
CELP str;
int mode;
{
    PORT *pp=DsFOpen(STRPpart(str),mode);
    return (pp)?DSPRTCEL(pp):NIL;
}


STATIC
CELP Ds_inport(port)
CELP port;
{
    GLOB(inport)=CELPRT(port);
    return port;
}

STATIC
CELP Ds_outport(port)
CELP port;
{
    GLOB(outport)=CELPRT(port);
    return port;
}

STATIC
CELP Ds_errport(port)
CELP port;
{
    GLOB(errport)=CELPRT(port);
    return port;
}


/*---------------------------- OS2 IO Speedup Code -------------------------*/
#ifdef OS2
#define DOSBUFSIZE 250
#define H(s) ((HDOS *)(s))
typedef struct _hdos
    {
        HFILE   handle;                    /* 2 */
        SHORT   bufpos;                    /* 2 */        
        CHAR    buffer[DOSBUFSIZE+2];    /* 252 */
    }   HDOS;                   /* total is 256 */

STATIC void PASCAL DsDosWrite(int c, UNTYPE *stream);
STATIC void PASCAL DsDosControl(UNTYPE *stream, int action);
/***************************************************************
** NAME:        DsDosOpen
** SYNOPSIS:    PORT *DsDosOpen(handle,mode);
**              int handle;     name of file to be opened.
**              int mode;     filemode
** DESCRIPTION: Opens a SCHEME port to a OS2 handle
**              mode can be READMODE, WRITMODE or READWRIT.
** RETURNS:     Return pointer to port.
**              If port can't be opened, NIL is returned.
***************************************************************/
STATIC PORT * PASCAL DsDosOpen(handle,mode)
HFILE handle;
USHORT mode;
{
    HDOS *hand;

    if ((hand=malloc(sizeof(HDOS)))==NULL)
        DSVERROR(ERRMEM);
    hand->bufpos=0;
    hand->handle=handle;
    if (!(mode & READMODE))
        return DsInitPort( mode, (UNTYPE *)hand,
                           DsEInput, DsDosWrite, DsDosControl);
    DSVERROR(ERRNOT);
}


/***************************************************************
** NAME:        DsDosWrite
** SYNOPSIS:    void DsDosWrite(stream,c)
**              UNTYPE *stream;   Pointer to file stream.
** DESCRIPTION: Called by DsOut for Dos handles.
** RETURNS:     void
***************************************************************/
STATIC void PASCAL DsDosWrite(c,stream)
int c;
UNTYPE *stream;
{
    if (c=='\n')
    {
        H(stream)->buffer[H(stream)->bufpos++]=(char)'\r';
        H(stream)->buffer[H(stream)->bufpos++]=(char)'\n';
        DsDosControl(stream,IOFLUSH);
    }
    else
    {
        H(stream)->buffer[H(stream)->bufpos++]=(char)c;
        if (H(stream)->bufpos==DOSBUFSIZE)                           /* full? */
            DsDosControl(stream,IOFLUSH);
    }
}


/***************************************************************
** NAME:        DsDosControl
** SYNOPSIS:    char DsDosControl(stream,action)
**              UNTYPE *stream; Pointer to file stream.
**              int action;     IOCLOSE or IOFLUSH;
** DESCRIPTION: Called by DsClosePort when a dos port is to be
**              closed. Called by DsFlushPort to flush the port.
** RETURNS:     void.
***************************************************************/
STATIC void PASCAL DsDosControl(stream, action)
UNTYPE *stream;
int action;
{
    static USHORT nwrit;
    switch(action)
    {
    case IOCLOSE:
        DosClose(H(stream)->handle);
        free(stream);
        break;

    case IOFLUSH:
        DosWrite( H(stream)->handle,
                  H(stream)->buffer,
                  H(stream)->bufpos,&nwrit);
        H(stream)->bufpos=0;
        break;
    }
}
#endif /* OS2 */
