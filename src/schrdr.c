/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHRDR.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/05
**
** DESCRIPTION: This module contains the DScheme parser engine.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schrdr.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHRDR.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHRDR.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:46:04   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"
#include "schdef.h"

#ifdef UNIX
    LONG strtol();
    REAL strtod();
#endif
#define EOL '\n'

#define INPCC          DsInput(inport)
#define TESTC          DsPeek(inport)
#define UNPCC(c)       DsUnput(inport,c)

#define EOFTOKEN    0
#define POSITIVE    1
#define NEGATIVE    2
#define DOT         3
#define PAROPEN     4
#define PARCLOSE    5
#define BRACKETPAR  6
#define SQUOTE      7
#define BACKQUOTE   8
#define COMMA       9
#define COMMAAT     10
#define STRING      11
#define SYM_KEY     12
#define TIMESTAMP   13
#define NUMBERT     14
#define BOOLEAN     15
#define CHARACTER   16
#define OBJECTID    17

static PORT *inport;                                    /* current input port */
static BYTE *p;                               /* Character pointer in bigbuf */
static BYTE chars[256];            /* Truth table white space, float and hex */
static CELP yylval;                         /* Lex puts found tokens in here! */
                                  /********************************************/
#define WHITECHAR  0x80           /* +---+---+---+---+----------------------+ */
#define FLOATCHAR  0x40           /* | W | F | N | O |                      | */
#define NUMBER     0x20           /* +---+---+---+---+----------------------+ */
#define OCTAL      0x10           /********************************************/
#define BLANK      0x08           /* Blank means character can be discarded */
#define WHITEC(c)  (chars[(BYTE)(c)] & WHITECHAR)
#define FLOATC(c)  (chars[(BYTE)(c)] & FLOATCHAR)
#define NUMC(c)    (chars[(BYTE)(c)] & NUMBER)
#define OCTALC(c)  (chars[(BYTE)(c)] & OCTAL)
#define BLANKC(c)  (chars[(BYTE)(c)] & BLANK)

void   PASCAL  DsSkipBlank   __((void));
CELP   PASCAL  DsReadList    __((void));
void   PASCAL  DsGetWord     __((void));
void   PASCAL  DsLError      __((int nr));
int    PASCAL  DsParser      __((void));
int    PASCAL  DsLex         __((void));
int    PASCAL  DsSpecial     __((void));
int    PASCAL  DsReadNumber  __((int base));
int    PASCAL  DsReadString  __((void));

#define DSLERROR(n)   STOP(DsLError(n))

/***************************************************************
** NAME:        DsIniParser
** SYNOPSIS:    void DsIniParser()
** DESCRIPTION: Initializes the parser system. The character
**              coding and hex value table is filled here.
** RETURNS:     void
***************************************************************/
void PASCAL DsIniParser()
{
    int i;
    memset(chars,0,256);

    for (i=0;i<10;i++)
         chars[i+'0'] = (BYTE) (FLOATCHAR | NUMBER | OCTAL);
    chars['8'] = (BYTE) (FLOATCHAR | NUMBER);
    chars['9'] = (BYTE) (FLOATCHAR | NUMBER);

    chars[255]  |= WHITECHAR;       /* set whitechar flag */
    chars[' ']  |= WHITECHAR|BLANK;
    chars['\f'] |= WHITECHAR|BLANK; /* formfeed */
    chars['\n'] |= WHITECHAR|BLANK; /* newline */
    chars['\r'] |= WHITECHAR|BLANK; /* return */
    chars[']']  |= WHITECHAR;
    chars[')']  |= WHITECHAR;
    chars['[']  |= WHITECHAR;
    chars['(']  |= WHITECHAR;
    chars[';']  |= WHITECHAR;  /* comment is also white space */
    chars['"']  |= WHITECHAR;

    chars['E']  |= FLOATCHAR;
    chars['e']  |= FLOATCHAR;
    chars['.']  |= FLOATCHAR;
    chars['+']  |= FLOATCHAR;
    chars['-']  |= FLOATCHAR;

    chars['#']  |= NUMBER;
}


/*****************************************************************
** NAME:        DsRead
** SYNOPSIS:    CELP DsRead(port)
**              PORT *port;
** DESCRIPTION: DsRead reads an item from the <port>.
** RETURNS:     Item read.
*****************************************************************/
CELP PASCAL DsRead(port)
PORT *port;
{
    if (GLOB(GCflag)) DsGarbageCollect(NIL);
    inport=port;
    DsSkipBlank();
    GLOB(errline)=inport->lineno;
    switch(DsParser())
    {
    case DOT:      DSERROR(ERRDOT,NIL);
    case PARCLOSE: DSERROR(ERRPARS,NIL);
    }
    return(yylval);
}

/*****************************************************************
** NAME:        DsSkipBlank
** SYNOPSIS:    void DsSkipBlank()
** DESCRIPTION: Skips all discardable blank until first
**              non-blank character. Comments are also skipped.
** RETURNS:     void
*****************************************************************/
void PASCAL DsSkipBlank()
{
    register int c;
    do
    { 
	c=INPCC;
        if (c==';')
            while ((c=INPCC)!=EOL)                /* Crunch until end of line */
                if (c==EOF) break;                     /* or an EOF! is found */
    } while (BLANKC(c));
    UNPCC(c);                            /* Put first nonblank character back */
}


/**************************************************************
** NAME:        DsLError
** SYNOPSIS:    DsLError(nr);
**              int nr;         The error number
** DESCRIPTION: DsLError is called by the parser to raise an
**              error. This function calls DsError to handle
**              the error.
** RETURNS:     void
**************************************************************/
void PASCAL DsLError(nr)
int nr;
{
    *p='\0';                                             /* mark end of input */
    DsGetWord();
    GLOB(errline)=inport->lineno;
    DSERROR(nr,(BIGBUF[0]=='\0'||nr==ERREOF)?NIL:DsStrCell(BIGBUF));
}

/**************************************************************
** NAME:        DsParser
** SYNOPSIS:    int DsParser()
** DESCRIPTION: DsParser parses a symbol of expression read
**              from inport.
** RETURNS:     TRUE, when symbol is completed.
**              FALSE, when EOF is encountered.
**              DOT, when a dot is found.
**              PARCLOSE, when a ')' or ']' is found.
**************************************************************/
int PASCAL DsParser()
{
    register int token;

    token=DsLex();
    switch(token)
    {
    case EOFTOKEN:  yylval=Q_eof;return FALSE;

    case POSITIVE:  if (DsLex()!=NUMBERT) DSLERROR(ERRNUM);
                    return TRUE;

    case NEGATIVE:  if (DsLex()!=NUMBERT) DSLERROR(ERRNUM);
                    switch(TAGpart(yylval))
                    {
                    case TYPE_INT:  INTpart(yylval) = -INTpart(yylval); break;
                    case TYPE_FLT:  yylval->dat.rv = -yylval->dat.rv; break;
                    default:        TAGpart(yylval)=TYPE_BIGN;         break;
                    }
                    return TRUE;

    case DOT:       return DOT;

    case PAROPEN:   yylval=DsReadList();
                    return TRUE;

    case PARCLOSE:  return PARCLOSE;

    case BRACKETPAR:yylval=DsReadList();
                    if (ISTRUE(yylval)) yylval=Ds_lstvec(yylval);
                    return TRUE;

    case SQUOTE:    if (DsParser()==TRUE)
                    {
                        yylval=LIST(Q_quote,yylval);
                        return TRUE;
                    }
                    else
                    {            
                        BIGBUF[0]='\0';
                        DSLERROR(ERRQUO);
                    }

    case BACKQUOTE:
    case COMMA:
    case COMMAAT:   DSLERROR(ERRNOT);

    case OBJECTID:
    case STRING:
    case SYM_KEY:
    case TIMESTAMP:
    case NUMBERT:
    case BOOLEAN:
    case CHARACTER: return TRUE;           /*** simple types ***/

    default: DSLERROR(ERRPINT);
    }
    return FALSE;			/* Avoids compiler warning */
}


/**************************************************************
** NAME:        DsReadList
** SYNOPSIS:    CELP DsReadList()
** DESCRIPTION: Readlist calls repeately the DsParser to read
**              in all the elements of the list until a
**              PARCLOSE is found.
** RETURNS:     The pointer to the new list.
**************************************************************/
CELP PASCAL DsReadList()
{
    CELP f,lp;

    f=lp=NIL;
    while (1)
    {
        switch (DsParser())
        {
        case EOFTOKEN:
            DSVERROR(ERREOF);

        case DOT:
            if (DsParser()==TRUE) /* next part */
            {
                if (ISTRUE(lp))
                    CDRpart(lp)=yylval;
                else
                    f=lp=yylval;
            }
            else
                DSERROR(ERRDOT,NIL);
            break;

        case TRUE:
            if (ISTRUE(lp))
                lp=CDRpart(lp)=DsCons1(yylval);
            else
                lp=f=DsCons1(yylval);
            break;

        case PARCLOSE:
            return(f);
        }
    }
}


/**************************************************************
** NAME:        DsLex
** SYNOPSIS:    int DsLex()
** DESCRIPTION: DsLex reads a token from inport.
** RETURNS:     An token number.
**************************************************************/
int PASCAL DsLex()
{
    int c;

    p=(BYTE*)BIGBUF;
    while (1)
    {
        switch(c=INPCC)
        {
        case EOF: return EOFTOKEN;                                     /* EOF */
        case ' ' :
        case '\n':
        case '\t':
        case '\r':
            break;

        case ';' :
            while((c=INPCC)!=EOL)                 /* Crunch until end of line */
                if (c==EOF)
                    return EOFTOKEN;                   /* or an EOF! is found */
            break;

        case '[' :
        case '(' :
            return PAROPEN;

        case ']' :
        case ')' :
            return PARCLOSE;

        case '.' :
            if (FLOATC(TESTC))
	    {
		*p++=(BYTE)c;
		DsGetWord();
                yylval=DsStrReal(BIGBUF);
                return NUMBER;
	    }
            return DOT;

        case '-' :
            if (NUMC(TESTC))          /* Look at value of next char in stream */
                return NEGATIVE;
	    goto symbol;

        case '+' :
            if (NUMC(TESTC))          /* Look at value of next char in stream */
                return(POSITIVE);
	    goto symbol;

        case '\'':
            return SQUOTE;

        case '`' :
            return BACKQUOTE;

        case ',' :
            if (TESTC!='@')
                return COMMA;
            else
            {
                INPCC;
                return COMMAAT;
            }

        case '#' :
            return DsSpecial();

        case '"' :
            return DsReadString();

        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
            *p++=(BYTE)c;
            return (DsReadNumber(10));

        default:
	symbol:
            *p++=(BYTE)c;
            DsGetWord();                            /* get remainder of token */
            yylval=DsSymbol(strupr(BIGBUF));              /* store identifier */
            return(SYM_KEY);                         /* Its a symbol or a key */
        }
    }
}


/**************************************************************
** NAME:        DsSpecial
** SYNOPSIS:    int DsSpecial()
** DESCRIPTION: DsSpecial reads a special symbol.
**              A special starts with '#'.
** RETURNS:     An token number.
**************************************************************/
int PASCAL DsSpecial()
{
    register int c=INPCC;

    switch(c)
    {
    case EOF:
        DSLERROR(ERREOF);

    case '!' :
        DsReadNumber(16);
        TAGpart(yylval)=TYPE_OID;
        return OBJECTID;

    case '@' :
        DsGetWord();
        TMSCEL(yylval,BIGBUF);
        return TIMESTAMP;

    case 'f' :
    case 'F' :
        yylval=Q_false;
        return BOOLEAN;

    case 't' :
    case 'T' :
        yylval=Q_true;
        return BOOLEAN;

    case 'b' :
    case 'B' :
        return(DsReadNumber(2));

    case 'o' :
    case 'O' :
        return(DsReadNumber(8));

    case 'd' :
    case 'D' :
        return(DsReadNumber(10));

    case 'x' :
    case 'X' :
        return(DsReadNumber(16));

    case '\\' :
        DsGetWord();                                     /* get rest of token */
        c=BIGBUF[0];
        if (c=='\0')      /* Revised report preferres #\space but some use #\ */
            c=' ';                                  /* See page 24 of r3.99rs */
        else
            if (BIGBUF[1]!='\0')/* string after #\ is longer than 1 character */
            {
                p=(BYTE *)strupr(BIGBUF);
                if (!strcmp(p,"SPACE"))          c=' ';
                else if (!strcmp(p,"NEWLINE"))   c='\n';
                else if (!strcmp(p,"TAB"))       c='\t';   /* These are extra */
                else if (!strcmp(p,"BELL"))      c=BELL;   /* These are extra */
                else if (!strcmp(p,"BACKSPACE")) c='\b';   /* These are extra */
                else if (!strcmp(p,"RETURN"))    c='\r';   /* These are extra */
                else if (!strcmp(p,"FORMFEED"))  c='\f';   /* These are extra */
                else if (!strcmp(p,"ESCAPE"))    c='\27';
                else DSLERROR(ERRCHAR);
            }
        CHRCEL(yylval,c);
        return(CHARACTER);

    case '[' :
    case '(' :
        return BRACKETPAR;

    default :
	*p++='#';
	*p++=(BYTE)c;
        DsGetWord();                              /* get remainder of token */
        yylval=DsSymbol(strupr(BIGBUF));        /* store identifier */
        return(SYM_KEY);                         /* Its a symbol or a key */
    }
}

                  
/**************************************************************
** NAME:        DsReadNumber
** SYNOPSIS:    int DsReadNumber(base)
**              int base;       base number of integer.
** DESCRIPTION: Reads an integer number.
** RETURNS:     An token number.
**************************************************************/
int PASCAL DsReadNumber(base)
int base;
{
    DsGetWord();                                 /* Get the rest of the token */
    yylval=DsStrNumber(BIGBUF,base);
    return NUMBERT;
}

CELP PASCAL DsStrNumber(str,base)
char *str;
int base;
{
    char *q;
    LONG res;

    if (str[0]=='#')
    {
	switch (toupper(str[1]))
	{
        case 'B': base=2;break;
	case 'O': base=8;break;
	case 'X': base=16;break;
	case 'D': base=10;break;
	default : DSLERROR(ERRCHARN);
	}
	str+=2;
    }
    if (base==10)
    {
        for (q=str; *q; q++)          /* Check if it has floating point chars */
            if (base==10 && (*q=='e' || *q=='E' || *q=='.'))
                return DsStrReal(str);
        if ((q-str)>9 && (GLOB(bignum)))             /* This is a BIG number! */
            return DsStrBig(str);
    }
    errno=0;
    res=strtol(str,&q,base);
    if (*q) DSLERROR(ERRCHARN);
    if (errno)                                           /* Overflow occurred */
    {
        if (base!=10) DSLERROR(ERRBASE);
        return DsStrReal(str);               /* Try to read it as a FP number */
    }
    return DSINTCEL(res);
}

                  
/**************************************************************
** NAME:        read_real
** SYNOPSIS:    int read_real()
** DESCRIPTION: read_real reads an floating point number.
** RETURNS:     An token number.
**************************************************************/
CELP PASCAL DsStrReal(s)
char *s;
{
    REAL r;

    errno=0;
    r=strtod(s,&p);                                             /* convert it */
    if (*p!=0 || errno)                /* not at end of string or RANGE error */
         DSERROR(ERRNUMF,NIL);
    return DSFLTCEL(r);                                      /* store in cell */
}


/**************************************************************
** NAME:        DsGetWord
** SYNOPSIS:    void DsGetWord();
** DESCRIPTION: This function reads the input, and stores it in
**              BIGBUF until a whitespace character is found.
** RETURNS:     void
**************************************************************/
void PASCAL DsGetWord()
{
    register int c;
    while (!WHITEC(c=INPCC))
	*p++=(BYTE)c;
    UNPCC(c);
    *p='\0';                                     /* Oops read some whitespace */
}


/**************************************************************
** NAME:        DsReadString
** SYNOPSIS:    int DsReadString();
** DESCRIPTION: This function reads a SCHEME string. Escaped
**              characters are \n (newline), \t (tab) and \\.
** RETURNS:     predefined token: STRING.
**************************************************************/
int PASCAL DsReadString()
{
    int c;
    int i=0;

    c=INPCC;
    while ((i<BIGMAX) && (c!='"'))
    {
        if (c==EOF) DSVERROR(ERREOF);
        if (c=='\\')                                           /* escape key! */
        {
            switch(c=INPCC)       
            {
            case 'a': c=BELL;break;                    /* or escape sequences */
            case 'b': c='\b';break;
            case 't': c='\t';break;
            case 'n': c='\n';break;
            case 'f': c='\f';break;
            case 'r': c='\r';break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7': i=(c-'0')<<6; c=INPCC;
	              if (!OCTALC(c)) DSLERROR(ERROCTAL);
	              i+=(c-'0')<<3;c=INPCC;
		      if (!OCTALC(c)) DSLERROR(ERROCTAL);
                      c+=i-'0';break;
            case EOF: DSVERROR(ERREOF);
            }
        }
        *p++=(BYTE)c;
        i++;
	c=INPCC;
    }
    *p= '\0';                                                /* end of string */
    if (c!='"')                       /* if not at end, crunch rest of string */
    {
        DSLERROR(ERRSTR);
        while ((c=INPCC)!='"')
        {
            if (c==EOF) break;
            if (c=='\\') INPCC;           /* discard char's after escape code */
        }
    }
    STRCEL(yylval,BIGBUF);
    return(STRING);
}
