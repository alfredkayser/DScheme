/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHSTR.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/12
**
** DESCRIPTION: All the Scheme STR functions are defined here.
**              If the format of the cel is changed only this
**              module needs to be changed.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schbuf.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHSTR.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHSTR.C_V  $
**
**                 Rev 1.0   12 Oct 1989 11:45:58   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"
#include "schdef.h"

#define PROTBIT 0x2
#define BUFGRAN 4

STATIC char * PASCAL DsGetBuf   __((int len));
STATIC CELP PASCAL DsGetVec __((int n));
STATIC CELP PASCAL DsBufUsed __((void));
STATIC CELP CDECL Ds_makestr    __((int n, CELP args));
STATIC CELP CDECL Ds_string     __((int n, CELP args));
STATIC CELP CDECL Ds_strlen     __((CELP q));
STATIC CELP CDECL Ds_strref     __((CELP str, CELP pos));
STATIC CELP CDECL Ds_strset     __((CELP str, CELP pos, CELP chr));
STATIC CELP CDECL Ds_streq      __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strlt      __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strle      __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strge      __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strgt      __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_streqci    __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strltci    __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strleci    __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strgeci    __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strgtci    __((CELP str1, CELP str2));
STATIC CELP CDECL Ds_strsub     __((CELP str, CELP pos, CELP cnt));
STATIC CELP CDECL Ds_strappend  __((CELP p, CELP q));
STATIC CELP CDECL Ds_strlist    __((CELP p));
STATIC CELP CDECL Ds_liststr    __((CELP p));
STATIC CELP CDECL Ds_strfill    __((CELP p, CELP q));
STATIC CELP CDECL Ds_strcopy    __((CELP p));

STATIC CELP CDECL Ds_intchar    __((CELP p));
STATIC CELP CDECL Ds_charint    __((CELP p));
STATIC CELP CDECL Ds_charup     __((CELP p));
STATIC CELP CDECL Ds_chardown   __((CELP p));
STATIC CELP CDECL Ds_intstr     __((int n, CELP p));
STATIC CELP CDECL Ds_strint     __((int n, CELP p));
STATIC CELP CDECL Ds_symstr     __((CELP q));
STATIC CELP CDECL Ds_strsym     __((CELP q));
STATIC CELP CDECL Ds_makevec    __((int n, CELP a));
STATIC CELP CDECL Ds_vector     __((int n, CELP l));
STATIC CELP CDECL Ds_veclen     __((CELP v));
STATIC CELP CDECL Ds_vecref     __((CELP v, CELP i));
STATIC CELP CDECL Ds_vecset     __((CELP v, CELP i, CELP a));
STATIC CELP CDECL Ds_veclst     __((CELP v));
STATIC CELP CDECL Ds_vecfill    __((CELP p, CELP q));
#ifdef DEBUG
STATIC CELP CDECL Ds_dumpstr    __((void));
#endif

STATIC
EXTDEF StringFunctions[] =
{
    /* Section 6.6 of Revised3.99 Scheme */
    {"INTEGER->CHAR",  (EXTPROC)Ds_intchar,   1, TYPE_INT},
    {"CHAR->INTEGER",  (EXTPROC)Ds_charint,   1, TYPE_CHR},
    {"CHAR-UPCASE",    (EXTPROC)Ds_charup,    1, TYPE_CHR},
    {"CHAR-DOWNCASE",  (EXTPROC)Ds_chardown,  1, TYPE_CHR},
    /* remaining functions (the xxx? procedures) are in schprc.h */

    /* Section 6.7 of Revised3.99 Scheme */
    {"MAKE-STRING",    (EXTPROC)Ds_makestr,  -1},
    {"STRING",         (EXTPROC)Ds_string,  -1},
    {"STRING-LENGTH",  (EXTPROC)Ds_strlen,    1, TYPE_STR},
    {"STRING-REF",     (EXTPROC)Ds_strref,    2, TYPE_STR, TYPE_INT},
    {"STRING-SET!",    (EXTPROC)Ds_strset,    3, TYPE_STR, TYPE_INT, TYPE_CHR},
    {"STRING=?",       (EXTPROC)Ds_streq,     2, TYPE_STR, TYPE_STR},
    {"STRING<?",       (EXTPROC)Ds_strlt,     2, TYPE_STR, TYPE_STR},
    {"STRING>?",       (EXTPROC)Ds_strgt,     2, TYPE_STR, TYPE_STR},
    {"STRING<=?",      (EXTPROC)Ds_strle,     2, TYPE_STR, TYPE_STR},
    {"STRING>=?",      (EXTPROC)Ds_strge,     2, TYPE_STR, TYPE_STR},
    {"STRING-CI=?",    (EXTPROC)Ds_streqci,   2, TYPE_STR, TYPE_STR},
    {"STRING-CI<?",    (EXTPROC)Ds_strltci,   2, TYPE_STR, TYPE_STR},
    {"STRING-CI>?",    (EXTPROC)Ds_strgtci,   2, TYPE_STR, TYPE_STR},
    {"STRING-CI<=?",   (EXTPROC)Ds_strleci,   2, TYPE_STR, TYPE_STR},
    {"STRING-CI>=?",   (EXTPROC)Ds_strgeci,   2, TYPE_STR, TYPE_STR},
    {"SUBSTRING",      (EXTPROC)Ds_strsub,    3, TYPE_STR, TYPE_INT, TYPE_INT},
    {"STRING-APPEND",  (EXTPROC)Ds_strappend, 2, TYPE_STR, TYPE_STR},
    {"STRING->LIST",   (EXTPROC)Ds_strlist,   1, TYPE_STR},
    {"LIST->STRING",   (EXTPROC)Ds_liststr,   1, 0},
    {"STRING-FILL!",   (EXTPROC)Ds_strfill,   2, TYPE_STR, TYPE_CHR},
    {"STRING-COPY",    (EXTPROC)Ds_strcopy,   1, TYPE_STR},
    {"NUMBER->STRING", (EXTPROC)Ds_intstr,   -1, 0},
    {"STRING->NUMBER", (EXTPROC)Ds_strint,   -1, 0},

    {"STRING->SYMBOL", (EXTPROC)Ds_strsym,    1, TYPE_STR},
    {"SYMBOL->STRING", (EXTPROC)Ds_symstr,    1, 0},

    /* Section 6.8 of Revised3.99 Scheme */
    {"MAKE-VECTOR",    (EXTPROC)Ds_makevec,  -1, 0},
    {"VECTOR",         (EXTPROC)Ds_vector,   -1, 0},
    {"VECTOR-LENGTH",  (EXTPROC)Ds_veclen,    1, TYPE_VEC},
    {"VECTOR-REF",     (EXTPROC)Ds_vecref,    2, TYPE_VEC, TYPE_INT},
    {"VECTOR-SET!",    (EXTPROC)Ds_vecset,    3, TYPE_VEC, TYPE_INT, 0},
    {"VECTOR->LIST",   (EXTPROC)Ds_veclst,    1, TYPE_VEC},
    {"LIST->VECTOR",   (EXTPROC)Ds_lstvec,    1, 0},         
    {"VECTOR-FILL!",   (EXTPROC)Ds_vecfill,   2, TYPE_STR, TYPE_CHR},
#ifdef DEBUG
    {"DUMP-STRINGS",   (EXTPROC)Ds_dumpstr,   0},
#endif
    {NULL}
};


/***************************************************************
** NAME:        DsIniBuf
** SYNOPSIS:    void DsIniBuf(size)
**              int size;       size of new buffer segment.
** DESCRIPTION: Allocates and initializes a new buffer segment.
** RETURNS:     void
***************************************************************/
void PASCAL DsIniBuf(size)
int size;
{
    char *buffer;
    CELP newstr;

    if (size>0)
    {
    	if (size%BUFGRAN) size+=BUFGRAN-(size%BUFGRAN);
        GETMEM(buffer,char,size,"Buffer Space");
        newstr=DsGetCell(TYPE_STR);
        PROTECT(newstr);             
        STRPpart(newstr)=buffer;
        STRLpart(newstr)=size;
        GLOB(freestr)=DsCons(newstr,GLOB(freestr)); 
        PROTECT(GLOB(freestr));

        newstr=DsGetCell(TYPE_STR);
        PROTECT(newstr); 
        STRPpart(newstr)=buffer;
        STRLpart(newstr)=size;
        GLOB(allostr)=DsCons(newstr,GLOB(allostr));
    	PROTECT(GLOB(allostr));              /* This descriptor is protected! */

        GLOB(strspace)+=(DWORD)size;
        GLOB(strsize)+=(DWORD)size;
    }
    DsFuncDef(StringFunctions); /* define string functions */
}


/***************************************************************
** NAME:        DsGetBuf
** SYNOPSIS:    char * DsGetBuf(len)
**              int len;    size of new string (including '\0')
** DESCRIPTION: DsGetBuf allocates a piece of string space to
**              store a string.
** RETURNS:     A pointer to a string space.
***************************************************************/
STATIC char * PASCAL DsGetBuf(len)
int len;
{
    CELP desc,p,pp;
    char *str;

    if (len%BUFGRAN) len+=BUFGRAN-(len%BUFGRAN);
    len+=sizeof(int);                         /* Reserve room for real length */
    pp=NIL;                 /* search for empty spot big enough to hold 'str' */
    for (p=GLOB(freestr); ISTRUE(p); p=CDRpart(p))
    {
        if (STRLpart(CARpart(p))>=len) break;     /* This block is big enough */
        pp=p;
    }
    if (ISNIL(p))                                            /* no fit found! */
        DSVERROR(ERRMEM);             /* string space empty or too fragmented */
    GLOB(strspace)-=len;             /* spot with len characters now reserved */
    if (GLOB(strspace)<256)             /* only a few chars left in strspace! */
        GLOB(GCflag)=1; /* G.C. asap (defragment if after GC still fragmented */
    desc=CARpart(p);
    str=STRPpart(desc);                      /* Pointer to fresh string space */
    STRLpart(desc)-=len;
    if (STRLpart(desc))                          /* remainder is still usable */
        STRPpart(desc)+=len;
    else                                      /* This free block is empty now */
    {   
        if (ISNIL(pp))
            GLOB(freestr)=CDRpart(p);        /* remove string desc. from list */
        else
            CDRpart(pp)=CDRpart(p);      /* remove string desc. from freelist */
        DsFreeCell(desc);                           /* remove this descriptor */
        DsFreeCell(p);                            /* Release this linker cell */
    }
    *(int *)str=len;        /* Store real length */
    return(str+sizeof(int));
}


/***************************************************************
** NAME:        DsFreeBuf
** SYNOPSIS:    void DsFreeBuf(buf);
**              char *buf;
** DESCRIPTION: DsFreeBuf frees the occupied space by the
**              buffer, by linking it to the freestr list.
**              It searches for a free block connected to it 
**              and extents that block so that it contains the
**              the new block. If there are two free blocks
**              connected (head and tail) then one freeblock
**              entry is destroyed and the other is extended.
**              This will defragment the string space after
**              a garbage collect.
** RETURNS:     void
***************************************************************/
void PASCAL DsFreeBuf(str)
char *str;
{
    CELP q, pr, bp=NIL, pp=NIL;
    int  chain=0, len;
    char *endstr;

    str-=sizeof(int);
    len=*(int *)str; 
    endstr=str+len;

    GLOB(strspace)+=len;
    for (q=GLOB(freestr); q; q=CDRpart(q))
    {
        pr=CARpart(q);
        if (!(chain&1) && (STRPpart(pr)+STRLpart(pr)==str))
        {                   /* Backchain i.e. chain freecell before str to it */
            if (!chain) bp=pr;            /* pointer to be changed descriptor */
            chain |= 1;                                  /* Performed a chain */
            str=STRPpart(pr);              /* try to chain this extended part */
            len+=STRLpart(pr);            /* New length of part to be chained */
        }
        else if (!(chain&2) && (STRPpart(pr)==endstr))
        {
            if (!chain) bp=pr;            /* pointer to be changed descriptor */
            chain |= 2;
            len+=STRLpart(pr);            /* New length of part to be chained */
        }
        if (chain==3)                                        /* Both chained! */
        {                                  /* Discard second changed freecell */
            if (ISTRUE(pp))
                CDRpart(pp)=CDRpart(q);
            else
                GLOB(freestr)=CDRpart(q);
            DsFreeCell(pr);
            DsFreeCell(q);     /* We can discard one link cell with its desc. */
            break;            /* Can't chain more so we break the search loop */
        }
        pp=q;
    }
    if (!chain)                        /* not chained! => prepend to freelist */
    {
    	bp=DsGetCell(TYPE_STR);
        STRPpart(bp)=str;
        STRLpart(bp)=len;
        PROTECT(bp);
        GLOB(freestr)=DsCons(bp,GLOB(freestr));     /* Add it to the freelist */
        PROTECT(GLOB(freestr));        /* Freestr linklist cell are protected */
    }
    else
    {
        STRPpart(bp)=str;               /* change entries of merged freecell. */
        STRLpart(bp)=len;
    }
}


/***************************************************************
** NAME:        DsFragmented
** SYNOPSIS:    int PASCAL DsFragmented();
** DESCRIPTION: Checks if the string space if fragmented.
**              It is fragmented unless there is at least one 
**              big free block.
** RETURNS:     TRUE, when string space if too much fragmented.
**              FALSE, string space is clean enough.
***************************************************************/
int PASCAL DsFragmented()
{
    CELP p;

    if (ISNIL(CDRpart(GLOB(freestr)))) return FALSE;
    for (p=GLOB(freestr); ISTRUE(p); p=CDRpart(p))
        if (STRLpart(CARpart(p))>256)  
            return FALSE;
    return TRUE;
}


STATIC CELP PASCAL DsBufUsed()
{
    CELP used,p,q;
    int i;

    DSGCmessage(GCrun);                                /* we're still running */
    used=NIL;
    for (p=GLOB(fblk); ISTRUE(p); p=CDRpart(p)) 
    {
        q=p;
        for (i=(int)CARIpart(p)-1;i>0;i--)
	{
	    q++;
            if (TAGpart(q)==TYPE_STR || TAGpart(q)==TYPE_VEC)
                if (!(GCFpart(q)&PROTBIT))    /* Not a protected descriptor */
                    used=DsCons(q,used);  /* Chain it to list of used strings */
        }
    }
    return used;
}


void PASCAL DsBufDefrag()
{
    CELP p, q, used;
    FILE *fp;

    used=DsBufUsed();
    DSGCmessage(GCrun);                                /* we're still running */
    fp=tmpfile();
    for (q=used; ISTRUE(q); q=CDRpart(q))    
    {
        p=CARpart(q);
        fwrite(STRPpart(p), STRLpart(p), 1, fp); /* write buffer into tmpfile */
        DsFreeBuf(STRPpart(p));                             /* Release buffer */
    }                                   /* All used space should be free now! */
    fseek(fp,0L,SEEK_SET);
    DSGCmessage(GCrun);                                /* we're still running */
    for (q=used; ISTRUE(q); q=p)    
    {
        p=CARpart(q);
        STRPpart(p)=DsGetBuf(STRLpart(p));          /* get fresh buffer space */
        fread(STRPpart(p), STRLpart(p), 1, fp);      /* read buffer data back */
        p=CDRpart(q);
        DsFreeCell(q);
    }
    fclose(fp); 
}


/***************************************************************
** NAME:        DsStrCell
** SYNOPSIS:    CELP DsStrCell(str)
**              char *str;      pointer to input string
** DESCRIPTION: Strcel converts a C string into a SCHEME string.
** RETURNS:     A pointer to a SCHEME string.
***************************************************************/
CELP PASCAL DsStrCell(str)
CONST char *str;
{
    CELP p;
    
    p=DsGetStr(strlen(str));
    strncpy(STRPpart(p),str,STRLpart(p));           /* fill it with its value */
    return(p);
}


/***************************************************************
** NAME:        DsCmpVector
** SYNOPSIS:    CELP DsCmpVector(a1,a2)
**              CELP a1, a2; the two vectors to compare.
** DESCRIPTION: Compares two vectors.
** RETURNS:     Q_true when equal, NIL otherwise.
***************************************************************/
CELP PASCAL DsCmpVector(a1,a2)
CELP a1,a2;
{
    CELP *v1, *v2;
    register int i;

    TYPCHECK(a1,TYPE_VEC);
    TYPCHECK(a2,TYPE_VEC);
    i=VECLpart(a1);
    if (i!=VECLpart(a2)) return NIL;
    v1=VECPpart(a1);
    v2=VECPpart(a2);
    while (i-->0)
	if (ISNIL(DsCompare(*v1++,*v2++))) 
	    return NIL;
    return Q_true;
}


STATIC
CELP Ds_makestr(n,args)
int n;
CELP args;
{
    if (n==2)
    {
        CELP q=CDRpart(args);
        TYPCHECK(q,TYPE_CHR);
        args=CARpart(args);
        TYPCHECK(args,TYPE_INT);
        return Ds_strfill(DsGetStr((int)INTpart(args)),q);
    }
    TYPCHECK(args,TYPE_INT);
    return DsGetStr((int)INTpart(args));
}

STATIC
CELP Ds_string(n,args)
int n;
CELP args;
{
    CELP p,c;
    int i;
    
    p=DsGetStr(n);
    for (i=0;i<n;i++)
    {
        if (i) args=CDRpart(args);
        c=CARpart(args);
        TYPCHECK(c,TYPE_CHR);
        STRPpart(p)[i]=(char)CHRpart(c);
    }
    STRPpart(p)[n]='\0';
    return p;
}


STATIC
CELP Ds_strfill(p,q)
CELP p,q;
{
    int i;
    for (i=0;i<STRLpart(p);i++)
        STRPpart(p)[i]=(char)CHRpart(q);
    return p;
}

STATIC
CELP Ds_strcopy(p)
CELP p;
{
    CELP q=DsGetStr(STRLpart(p));
    strncpy(STRPpart(q),STRPpart(p),STRLpart(p));
    return q;
}

STATIC
CELP Ds_charint(arg)
CELP arg;
{
    CELP p;
    INTCEL(p,CELCHR(arg)); return p;
}


STATIC CELP Ds_intchar(arg)
CELP arg;
{
    return DSCHRCEL((char)CELINT(arg));
}

STATIC CELP Ds_charup(arg)
CELP arg;
{
    return DSCHRCEL(toupper((char)CHRpart(arg)));
}

STATIC CELP Ds_chardown(arg)
CELP arg;
{
    return DSCHRCEL(tolower((char)CHRpart(arg)));
}

STATIC CELP Ds_strint(n, arg)
int n;
CELP arg;
{
    int base;
    switch(n)
    {
    case 1:base=10;break;
    case 2:TYPCHECK(CDRpart(arg),TYPE_INT);
	   base=INTpart(CDRpart(arg));
	   arg=CARpart(arg);
	   break;
    default:DSERROR(ERRARC,arg);
    }
    TYPCHECK(arg,TYPE_STR);
    return DsStrNumber(STRPpart(arg),base);
}


STATIC CELP Ds_intstr(n,arg)
int n;
CELP arg;
{
    int base;
    switch(n)
    {
    case 1:base=10;break;
    case 2:TYPCHECK(CDRpart(arg),TYPE_INT);
	   base=INTpart(CDRpart(arg));
	   arg=CARpart(arg);
	   break;
    default:DSERROR(ERRARC,arg);
    }
    switch(TAG(arg))
    {
    case TYPE_INT: 
	if (base<2 || base>36) DSERROR(ERRARC,arg);
        return DsStrCell(ltostr(INTpart(arg),base));

    case TYPE_BIGN:
    case TYPE_BIGP:
	if (base!=10) DSERROR(ERRBASE,arg);
        return DsStrCell(DsBigStr(arg));

    case TYPE_FLT:
	if (base!=10) DSERROR(ERRBASE,arg);
        sprintf(BIGBUF,"%g",FLTpart(arg));
        return DsStrCell(BIGBUF);
    }
    DSTERROR(arg);
}

STATIC
CELP Ds_strlist(s)
CELP s;
{
    CELP cp;
    char *sp;
    register int i;

    cp=NIL;
    sp=STRPpart(s)+STRLpart(s);
    for (i=STRLpart(s);i>0;i--)
        cp=DsCons(DSCHRCEL(*--sp),cp);
    return(cp);
}


STATIC
CELP Ds_liststr(p)
CELP p;
{
    char *str;
    CELP q,np;
    SHORT cnt;
    
    np=DsGetStr(cnt=(SHORT)DsLength(p));
    str=STRPpart(np);
    while (cnt-->0)
    {
        TYPCHECK(p,TYPE_PAIR);
        q=CARpart(p);
        TYPCHECK(q,TYPE_CHR);
        *str++=(char)CELCHR(q);
        p=CDRpart(p);
    }
    if (!ISNIL(p)) DSTERROR(p);
    return(np);
}


/***************************************************************
** NAME:        Ds_strappend
** SYNOPSIS:    CELP Ds_strappend(p,q)
**              CELP p;         DScheme string
**              CELP q;         DScheme string
** DESCRIPTION: Ds_strappend combines two strings into one string
** RETURNS:     A DScheme string
***************************************************************/
STATIC
CELP Ds_strappend(p,q)
CELP p,q;
{
    CELP np;
    char *s;
    int cnt;

    cnt=STRLpart(p)+STRLpart(q);              /* we can discard one '\0' byte */
    np=DsGetStr(cnt);                                       /* Get fresh cell */
    s=STRPpart(np);
    strncpy(s,STRPpart(p),STRLpart(p));
    strncpy(s+STRLpart(p),STRPpart(q),STRLpart(q));     /* Append second part */
    return(np);
}


STATIC
CELP Ds_symstr(q)
CELP q;
{
    if (!ISSYM(q)) DSTERROR(q);
    return(CDRpart(q));
}


STATIC
CELP Ds_strsym(q)
CELP q;
{
    return(DsSymbol(STRPpart(q)));
}

STATIC
CELP Ds_strlen(q)
CELP q;
{
    CELP p;
    INTCEL(p,(STRLpart(q)));
    return p;
}

STATIC
CELP Ds_strset(str,pos,chr)
CELP str,pos,chr;
{
    register int i=(int)CELINT(pos);
    if (i<0 || i>=STRLpart(str)) return(NIL);
    STRPpart(str)[i]=(char)CELCHR(chr);                          /* modify it */
    return(str);                                   /* returns modified string */
}

STATIC
CELP Ds_strref(str,pos)
CELP str;
CELP pos;
{
    register int i=(int)CELINT(pos);
    if (i<0 || i>=STRLpart(str)) return(NIL);
    return DSCHRCEL(STRPpart(str)[i]);
}

STATIC
CELP Ds_strsub(str,pos,cnt)
CELP str,pos,cnt;
{
    CELP np;
    int i,l;

    i=(int)CELINT(pos);
    l=(int)CELINT(cnt);
    if (! (0<=i && i<=l && l<=STRLpart(str)))               /* Bound checking */
        return(NIL);                         
    l-=i;                                              /* length of substring */
    np=DsGetStr(l);
    strncpy(STRPpart(np),STRPpart(str)+i,l);              /* add start offset */
    return(np);
}

STATIC
CELP Ds_strlt(str1,str2)
CELP str1,str2;
{
    RETBOO(strcmp(STRPpart(str1),STRPpart(str2)) <  0);
}

STATIC
CELP Ds_strle(str1,str2)
CELP str1,str2;
{
    RETBOO(strcmp(STRPpart(str1),STRPpart(str2)) <= 0);
}

STATIC
CELP Ds_streq(str1,str2)
CELP str1,str2;
{
    RETBOO(strcmp(STRPpart(str1),STRPpart(str2)) == 0);
}

STATIC
CELP Ds_strge(str1,str2)
CELP str1,str2;
{
    RETBOO(strcmp(STRPpart(str1),STRPpart(str2)) >= 0);
}

STATIC
CELP Ds_strgt(str1,str2)
CELP str1,str2;
{
    RETBOO(strcmp(STRPpart(str1),STRPpart(str2)) >  0);
}

STATIC
CELP Ds_strltci(str1,str2)
CELP str1,str2;
{
    RETBOO(stricmp(STRPpart(str1),STRPpart(str2)) <  0);
}

STATIC
CELP Ds_strleci(str1,str2)
CELP str1,str2;
{
    RETBOO(stricmp(STRPpart(str1),STRPpart(str2)) <= 0);
}

STATIC
CELP Ds_streqci(str1,str2)
CELP str1,str2;
{
    RETBOO(stricmp(STRPpart(str1),STRPpart(str2)) == 0);
}

STATIC
CELP Ds_strgeci(str1,str2)
CELP str1,str2;
{
    RETBOO(stricmp(STRPpart(str1),STRPpart(str2)) >= 0);
}

STATIC
CELP Ds_strgtci(str1,str2)
CELP str1,str2;
{
    RETBOO(stricmp(STRPpart(str1),STRPpart(str2)) >  0);
}

STATIC CELP PASCAL DsGetVec(n)
int n;
{
    CELP p=DsGetCell(TYPE_VEC);
    STRLpart(p)=n*sizeof(CELP);
    STRPpart(p)=DsGetBuf(n*sizeof(CELP));
    return p;
}

CELP PASCAL DsGetStr(n)
int n;
{
    CELP p=DsGetCell(TYPE_STR);
    STRLpart(p)=n;
    STRPpart(p)=DsGetBuf(n+1);
    STRPpart(p)[n]='\0';
    return p;
}



STATIC
CELP Ds_makevec(n,args)
int n;
CELP args;
{
    CELP q=NIL;

    if (n==2)
    {
        q=CDRpart(args);
        args=CARpart(args);
    }
    return Ds_vecfill(DsGetVec((int)INTpart(args)),q);
}

STATIC
CELP Ds_vector(n,args)
int n;
CELP args;
{
    CELP p;
    int i;
    
    p=DsGetVec(n);
    for (i=0;i<n;i++)
    {
        if (i) args=CDRpart(args);
        VECPpart(p)[i]=CARpart(args);
    }
    return p;
}

STATIC
CELP Ds_veclen(v)
CELP v;
{
    CELP p;
    INTCEL(p,(VECLpart(v)));
    return p;
}

STATIC
CELP Ds_vecref(vec, pos)
CELP vec, pos;
{
    register int i=(int)INTpart(pos);
    if (i<0 || i>=VECLpart(vec)) return(NIL);
    return VECPpart(vec)[i];
}

CELP Ds_vecset(vec,pos,arg)
CELP vec,pos,arg;
{
    register int i=(int)CELINT(pos);
    if (i<0 || i>=VECLpart(vec)) return(NIL);
    VECPpart(vec)[i]=arg;                          /* modify it */
    return(vec);                                   /* returns modified string */
}



STATIC
CELP Ds_veclst(s)
CELP s;
{
    CELP cp, *vp;
    register int i=VECLpart(s);

    cp=NIL;
    vp=VECPpart(s);
    while (i>0)
        cp=DsCons(vp[--i],cp);
    return(cp);
}


CELP Ds_lstvec(p)
CELP p;
{
    CELP *vp, np;
    int cnt;
    
    np=DsGetVec(cnt=(int)DsLength(p));
    vp=VECPpart(np);
    while (cnt-->0)
    {
        TYPCHECK(p,TYPE_PAIR);
        *vp++=CARpart(p);
        p=CDRpart(p);
    }
    return(np);
}

STATIC
CELP Ds_vecfill(p,q)
CELP p,q;
{
    int i;
    for (i=0;i<VECLpart(p);i++)
        VECPpart(p)[i]=q;
    return p;
}

#ifdef DEBUG
STATIC 
CELP Ds_dumpstr()
{
    CELP p,q;
    int i=0;
    long totlen=0L;

    DsOuts(GLOB(errport),"**********************************\n");
    DsOuts(GLOB(errport),"** Blok Address   Length EndOfBlok\n");
    DsOuts(GLOB(errport),"** ---- --------- ------ ---------\n");
    for (q=GLOB(freestr); ISTRUE(q); q=CDRpart(q))
    {
        p=CARpart(q);
        sprintf(GLOB(bigbuf),"** %-4d %09p %6d %09p\n",
             ++i, STRPpart(p), STRLpart(p), STRPpart(p)+STRLpart(p));
        DsOuts(GLOB(errport),GLOB(bigbuf));
        totlen+=STRLpart(p);
    }
    DsOuts(GLOB(errport),"**\n");
    DsOutf(GLOB(errport),"** %d part%s, %l bytes\n", i, i==1?"":"s", totlen);
    DsOuts(GLOB(errport),"**********************************\n");
    return Q_invis;
}
#endif
