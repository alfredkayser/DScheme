/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHCEL.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/10/12
**
** DESCRIPTION: All the Scheme CEL functions are defined here.
**              If the format of the cel is changed only these
**              function need to be changed.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision$
** CHANGER:     $Author$
** WORKFILE:    $Workfile$
** LOGFILE:     $Logfile$
** LOGINFO:     $Log$
**********************************************************************/
#include "schinc.h"
#include "schdef.h"

STATIC int PASCAL DsHash  __((CONST char *str));
STATIC CELP CDECL DsListRef  __((CELP list,CELP num));
STATIC CELP CDECL DsListTail __((CELP list,CELP num));
STATIC CELP CDECL DsAppend   __((CELP list,CELP num));
STATIC CELP CDECL Ds_Append  __((CELP list,CELP num));
STATIC CELP CDECL Ds_length  __((CELP list));
STATIC CELP CDECL Ds_reverse __((CELP list));

#define PROTBIT  0x2
#define MARKBIT  0x1
#define SETMARK(cel)  (GCFpart(cel) |= MARKBIT)     /* SET BIT 0 */
#define UNMARK(cel)   (GCFpart(cel) &= 0xfe)        /* RESET BIT 0 */
#define MARKED(cel)   (GCFpart(cel))                /* TEST GC BITS */

STATIC
EXTDEF CelFunctions[] =
    {
        {"LIST-REF",  (EXTPROC)DsListRef,  2, TYPE_PAIR, TYPE_INT},
        {"LIST-TAIL", (EXTPROC)DsListTail, 2, TYPE_PAIR, TYPE_INT},
        {"APPEND!",   (EXTPROC)DsAppend,   2, 0, 0},
        {"APPEND",    (EXTPROC)Ds_Append,  2, 0, 0},
        {"LENGTH",    (EXTPROC)Ds_length,  1, 0},
        {"REVERSE",   (EXTPROC)Ds_reverse, 1, 0},

        {"CAR",    (EXTPROC)DsCar,    1, 0},
        {"CAR",    (EXTPROC)DsCar,    1, 0},
        {"CDR",    (EXTPROC)DsCdr,    1, 0},
        {"CAAR",   (EXTPROC)DsCaar,   1, 0},
        {"CADR",   (EXTPROC)DsCadr,   1, 0},
        {"CDAR",   (EXTPROC)DsCdar,   1, 0},
        {"CDDR",   (EXTPROC)DsCddr,   1, 0},
        {"CAAAR",  (EXTPROC)DsCaaar,  1, 0},
        {"CAADR",  (EXTPROC)DsCaadr,  1, 0},
        {"CADAR",  (EXTPROC)DsCadar,  1, 0},
        {"CADDR",  (EXTPROC)DsCaddr,  1, 0},
        {"CDAAR",  (EXTPROC)DsCdaar,  1, 0},
        {"CDADR",  (EXTPROC)DsCdadr,  1, 0},
        {"CDDAR",  (EXTPROC)DsCddar,  1, 0},
        {"CDDDR",  (EXTPROC)DsCdddr,  1, 0},
        ENDOFLIST
    };

#ifdef MSDOS
void FAR * PASCAL DsFarMalloc(size)
int size;
{
    void FAR *ptr;
    struct SREGS segregs;

    ptr=_fmalloc(size);
    if (ISTRUE(ptr))
    {
        segread(&segregs);
        if (segregs.ds==FP_SEG(ptr))        /* don't allocate in data segment */
        {
            _ffree(ptr);
            ptr=NIL;
        }
    }
    return(ptr);
}
#endif


/*****************************************************************
** NAME:        DsIniCells
** SYNOPSIS:    void DsIniCells(numcels)
**              DWORD numcels;
** DESCRIPTION: Initializes the cell space.
** RETURNS:     void
*****************************************************************/
void PASCAL DsIniCells(numcels)
DWORD numcels;
{
    GLOB(GCtreshold) = (numcels<25600L) ? 256L : numcels/100L;
    while (numcels>0L)
    {
        DWORD c = min(numcels,6550L);
        DsAllocCells((int)c);
        numcels-=c;
    }
    DsFuncDef(CelFunctions);
}


/***************************************************************
** NAME:        DsAllocCells
** SYNOPSIS:    void DsAllocCells(size)
**              int size;       Number of cells to allocate
** DESCRIPTION: Allocates an array of new cells. The first cell
**              is used as a link to previously allocated arrays
**              and contains also the array size. The remaining
**              (size less one) cells are linked into the free
**              list. Global variables freecel and GCtreshold
**              are adjusted.
** RETURNS:     void
***************************************************************/
void PASCAL DsAllocCells(blocksize)
int blocksize;
{
    CELP p, array;
    int i;

    array = FARMALLOC(blocksize,CEL);
    if (ISNIL(array))
        DsMemError("Cell space");
    p = array;                                   /* Address of first new cell */
    TAGpart(p) = TYPE_SPC;                                    /* Default type */
    PROTECT(p);                                    /* set GC flags: protected */
    CARIpart(p) = blocksize;                            /* Remember blocksize */
    CDRpart(p) = GLOB(fblk);                     /* Chain to other cellblocks */
    for (i=1;i<blocksize;i++)                      /* For all cels in a block */
    {                                            /* Except the first and last */
          p++;                                       /* To next cell in block */
          TAGpart(p) = TYPE_FREE;                     /* Free type by default */
          CARpart(p) = NIL;                                  /* Reset to zero */
          CDRpart(p) = p+1;                       /* Point to next free block */
    }                                                   /* Until all is reset */
    CDRpart(p) = GLOB(freel);        /* Modify last cell to point to freelist */
    GLOB(fblk) = array;                             /* Remember pos. of array */
    GLOB(freel) = array+1;                          /* First cell is not free */
    GLOB(freecels) += blocksize-1;                   /* Number of fresh cells */
}


/***************************************************************
** NAME:        DsGetCell
** SYNOPSIS:    CELP DsGetCell()
** DESCRIPTION: Allocates a new cel. It takes a cel from the
**              free list and returns it. If the freelist is
**              empty the garbage collector is called in. If the
**              garbage collector can't find any free space, an
**              error is raised (ERRMEM).
** RETURNS:     Pointer to new cel.
** USES:        DsGarbageCollect, error
***************************************************************/
CELP PASCAL DsGetCell(typ)
int typ;
{
    register CELP temp;

    temp = GLOB(freel);                             /* First free cel in list */
    if (ISNIL(temp))
        DSVERROR(ERRMEM);                         /* No free cells? => error! */
    GLOB(freel) = CDRpart(temp);                /* Remove cell from free list */
    GLOB(freecels)--;
    if (GLOB(freecels)<GLOB(GCtreshold)) GLOB(GCflag)=TRUE;
    TAGpart(temp) = (BYTE)typ;
    GCFpart(temp) = 0;
    CDRpart(temp) = NIL;                          /* unlink from freecel list */
    return(temp);
}


/***************************************************************
** NAME:        DsFreeCell
** SYNOPSIS:    DsFreeCell(item)
**              CELP item       The cel to be released.
** DESCRIPTION: Releases a cel. In other words the cel is linked
**              into the freelist.
***************************************************************/
void PASCAL DsFreeCell(temp)
CELP temp;
{
    if (ISTRUE(temp) && TAGpart(temp)!=TYPE_FREE)          /* oke to release? */
    {
        TAGpart(temp) = TYPE_FREE;                              /* Reset type */
        GCFpart(temp) = 0;                                   /* Reset GC flag */
        CARpart(temp) = NIL;                                   /* Reset value */
        CDRpart(temp) = GLOB(freel);            /* Chain in front of freelist */
        GLOB(freel) = temp;
        GLOB(freecels)++;
    }
#ifdef DEBUG
    else
        DsOuts(GLOB(errport),"**** SERIOUS ERROR: Attempted to release free cell\n");
#endif
}


/***************************************************************
** NAME:        DsCons
** SYNOPSIS:    CELP DsCons(A,B);
**              CELP A,B        Pointers to the two pair items
** DESCRIPTION: Allocates a new cel and initializes it as a pair
**              with the pointers to A and B.
** RETURNS:     Pointer to the new pair cel.
***************************************************************/
CELP PASCAL DsCons(a,b)
CELP a,b;
{
    register CELP temp;

    temp = GLOB(freel);                             /* First free cel in list */
    if (ISNIL(temp))
        DSVERROR(ERRMEM);                         /* No free cells? => error! */
    GLOB(freel) = CDRpart(temp);                /* Remove cell from free list */
    GLOB(freecels)--;
    if (GLOB(freecels)<GLOB(GCtreshold))
        GLOB(GCflag)=TRUE;
    TAGpart(temp)=TYPE_PAIR;
    GCFpart(temp)=0;
    CARpart(temp)=a;
    CDRpart(temp)=b;
    return(temp);
}


/***************************************************************
** NAME:        DsCons1
** SYNOPSIS:    CELP DsCons1(A);
**              CELP A          Pointer to the car value.
** DESCRIPTION: Allocates a new cel and initializes it as a pair
**              with the car set to A and the cdr set to NIL.
** RETURNS:     Pointer to the new pair cel.
***************************************************************/
CELP PASCAL DsCons1(a)
CELP a;
{
    register CELP temp;

    temp = GLOB(freel);                             /* First free cel in list */
    if (ISNIL(temp))
        DSVERROR(ERRMEM);                         /* No free cells? => error! */
    GLOB(freel) = CDRpart(temp);                /* Remove cell from free list */
    GLOB(freecels)--;
    if (GLOB(freecels)<GLOB(GCtreshold))
        GLOB(GCflag)=TRUE;
    TAGpart(temp)=TYPE_PAIR;
    GCFpart(temp)=0;
    CARpart(temp)=a;
    CDRpart(temp)=NIL; 
    return(temp);
}


CELP PASCAL DsIntCell(l)
LONG l;
{
    CELP p=DsGetCell(TYPE_INT);
    INTpart(p)=l;
    return p;
}


CELP PASCAL DsFltCell(v)
REAL v;
{
    CELP p=DsGetCell(TYPE_FLT);
    FLTpart(p)=v;
    return p;
}


CELP PASCAL DsChrCell(c)
int c;
{
    CELP p=DsGetCell(TYPE_CHR);
    CHRpart(p)=c;
    return p;
}


CELP PASCAL DsPrtCell(prt)
PORT *prt;
{
    CELP p=DsGetCell(TYPE_PRT);
    PRTpart(p)=prt;
    return p;
}


/***************************************************************
** NAME:        DsHash
** SYNOPSIS:    int DsHash(str);
**              CONST char *str;      pointer to input string
** DESCRIPTION: Hash returns a hash number calculated by hashing
**              the input string and taking the modulo hashsize.
** RETURNS:     An integer between 0 and glo->hashsize.
***************************************************************/
STATIC
int PASCAL DsHash(str)
CONST char *str;
{
    register int j=0;
    while (*str)
         j = (j*3 + *str++) % GLOB(hashsize);
    return(j);
}


/***************************************************************
** NAME:        DsSymbol
** SYNOPSIS:    CELP DsSymbol(symstr)
**              char *symbol;   pointer to input string
** DESCRIPTION: DsSymbol searches for symstr in the symbol
**              table. If it is found the pointer to it is
**              returned, otherwise a new entry is made and
**              filled with the symbol string.
** RETURNS:     Pointer to existing or new cel containing symbol
***************************************************************/
CELP PASCAL DsSymbol(symbol)
CONST char *symbol;
{
    int i;
    int symlen;
    CELP p;

    i=DsHash(symbol);      /* p points to the first elem of list with symbols */
    symlen=strlen(symbol);                          /* with the same hashcode */

    for (p=GLOB(hashtab[i]); ISTRUE(p); p=CDRpart(p))
         if ( (STRLpart(CDARpart(p))==symlen)        /* compare stringlengths */
           && (strncmp(STRPpart(CDARpart(p)),symbol,symlen)==0))   /* compare */
              return(CARpart(p));                                /* Found it! */
    p = DsCons(NIL,DsStrCell(symbol));              /* combine "name" and NIL */
    TAGpart(p) = TYPE_SYM;                             /* default symbol type */
    GLOB(hashtab[i]) = DsCons(p,GLOB(hashtab[i])); /* link to old symbol list */
    return(p);                                    /* return ptr to symbol cel */
}


/***************************************************************
** NAME:        DsInsExt
** SYNOPSIS:    CELP DsInsExt(symstr)
**              char *symbol;   pointer to input string
** DESCRIPTION: Insertext searches for symstr in the symbol
**              table. If it is found the pointer to it is
**              returned, otherwise a new entry is made and
**              filled with the symbol string.
** RETURNS:     Pointer to existing or new cel containing symbol
***************************************************************/
CELP PASCAL DsInsExt(name)
CONST char *name;
{
    int i;
    int symlen;
    CELP p;

    i=DsHash(name);          /* p points to the first elem of list with names */
    symlen=strlen(name);                            /* with the same hashcode */

    for (p=GLOB(hashtab[i]); ISTRUE(p); p=CDRpart(p))
         if ( (STRLpart(CDARpart(p))==symlen)        /* compare stringlengths */
           && (strncmp(STRPpart(CDARpart(p)),name,symlen)==0))     /* compare */
              return(CARpart(p));                                /* Found it! */
    p = DsGetCell(TYPE_STR);                          /* create a string cell */
    PROTECT(p);
    STRPpart(p) = (char *)name;
    STRLpart(p) = strlen(name);
    p = DsCons(NIL,p);                              /* combine "name" and NIL */
    TAGpart(p)=TYPE_SYM;                                      /* default type */
    GLOB(hashtab[i]) = DsCons(p,GLOB(hashtab[i])); /* link to old symbol list */
    return(p);                                    /* return ptr to symbol cel */
}


/**************************************************************
** NAME:        DsListRef
** SYNOPSIS:    CELP DsListRef(list, num);
**              CELP list;
**              CELP num;
** DESCRIPTION: Returns the num-th element of the list.
** RETURNS:     Num-the list element.
**************************************************************/
STATIC
CELP DsListRef(list,num)
CELP list,num;
{
    list=DsListTail(list,num);
    return ISNIL(list) ? NIL : CARpart(list);
}


/**************************************************************
** NAME:        DsListTail
** SYNOPSIS:    CELP DsListTail(list, num);
**              CELP list;
**              CELP num;
** DESCRIPTION: Returns the tail after the num-th element of
**              the list.
** RETURNS:     Num-the list element.
**************************************************************/
STATIC
CELP DsListTail(list,num)
CELP list,num;
{
    int n;

    for (n=(int)INTpart(num); n>0; n--)
    {
        if (ISNIL(list)) break;
        list=DsCdr(list);
    }
    return list;
}


/**************************************************************
** NAME:        DsAppend
** SYNOPSIS:    CELP DsAppend(list, p);
**              CELP list;
**              CELP p;
** DESCRIPTION: Appends p to the end of the list.
** RETURNS:     Same list with p appended.
**************************************************************/
STATIC
CELP DsAppend(list,p)
CELP list,p;
{
    CELP q,pq;

    if (ISNIL(list)) return p;
    pq=list;
    q=DsCdr(pq);
    while (ISTRUE(q))
	q=DsCdr(pq=q);
    CDRpart(pq)=p;
    return list;
}

STATIC
CELP Ds_Append(list,p)
CELP list,p;
{
    CELP pq,q,np,fp;

    pq=list;
    if (ISNIL(list)) return p;
    q=DsCdr(pq);
    np=fp=DsGetCell(TYPE_PAIR);
    CARpart(np)=CARpart(pq);
    while (ISTRUE(q))
    {
	q=DsCdr(pq=q);
	np=CDRpart(np)=DsGetCell(TYPE_PAIR);
	CARpart(np)=CARpart(pq);
    }
    CDRpart(np)=p;
    return fp;
}

/***************************************************************
** NAME:        Ds_length
** SYNOPSIS:    CELP Ds_length(list)
**              CELP list;      List
** DESCRIPTION: Calculates the length of the list
** RETURNS:     The length.
***************************************************************/
CELP Ds_length(l)
CELP l;
{
    return DSINTCEL(DsLength(l));
}


/***************************************************************
** NAME:        Ds_reverse
** SYNOPSIS:    CELP Ds_reverse(list)
**              CELP list;      list to be reversed
** DESCRIPTION: Reverses the list.
** RETURNS:     The reversed list.
***************************************************************/
STATIC
CELP Ds_reverse(list)
CELP list;
{
    CELP cp=NIL;
    for ( ;ISTRUE(list); list=CDRpart(list))
        cp=DsCons(DsCar(list),cp);
    return(cp);
}


/***************************************************************
** NAME:        DsCar, DsCdr, ..., DsCdddr
** SYNOPSIS:    CELP DsCar(A);
**              CELP A;             Pointers to a list item.
** DESCRIPTION: These work all like the SCHEME functions CAR,
**              CDR, etc. When the car or cdr is taken from an
**              atom, the error function is called. The
**              arguments of car and cdr may be a pair, lambda
**              procedure or macro definitions.
** EXAMPLES:    (car (1 2 3))               ==> 1
**              (cdr (1 2 3))               ==> (2 3)
**              (define sum
**                 (lambda (x y) (+ x y)))  ==> #PROC
**              (car sum)                   ==> [(x . y), 2, 0]
**              (cadr sum)                  ==> ((+ x y))
**              (cddr sum)                  ==> proc.environment
** RETURNS:     Pointer to the car or cdr.
***************************************************************/
#define getCAR(l)  ISCAR(l)?(l)->dat.pair.car:DSERROR(ERRCAR,l)
#define getCDR(l)  ISCAR(l)?(l)->dat.pair.cdr:DSERROR(ERRCDR,l)
#define testCAR(l) if (!ISCAR(l)) DSERROR(ERRCAR,l)
#define testCDR(l) if (!ISCAR(l)) DSERROR(ERRCDR,l)

CELP DsCar(l)
CELP l;
{
    testCAR(l);
    return(CARpart(l));
}

CELP DsCdr(l)
CELP l;
{
    testCDR(l);
    return(CDRpart(l));
}

CELP DsCaar(l)
CELP l;
{
    testCAR(l);
    l=CARpart(l);
    testCAR(l);
    return(CARpart(l));
}

CELP DsCadr(l)
CELP l;
{
    testCDR(l);
    l=CDRpart(l);
    testCAR(l);
    return(CARpart(l));
}

CELP DsCdar(l)
CELP l;
{
    testCAR(l);
    l=CARpart(l);
    testCDR(l);
    return(CDRpart(l));
}

CELP DsCddr(l)
CELP l;
{
    testCDR(l);
    l=CDRpart(l);
    testCDR(l);
    return(CDRpart(l));
}

CELP DsCaaar(l)
CELP l;
{
    return(DsCar(DsCaar(l)));
}

CELP DsCaadr(l)
CELP l;
{
    return(DsCar(DsCadr(l)));
}

CELP DsCadar(l)
CELP l;
{
    return(DsCar(DsCdar(l)));
}

CELP DsCaddr(l)
CELP l;
{
    return(DsCar(DsCddr(l)));
}

CELP DsCdaar(l)
CELP l;
{
    return(DsCdr(DsCaar(l)));
}

CELP DsCdadr(l)
CELP l;
{
    return(DsCdr(DsCadr(l)));
}

CELP DsCddar(l)
CELP l;
{
    return(DsCdr(DsCdar(l)));
}

CELP DsCdddr(l)
CELP l;
{
    return(DsCdr(DsCddr(l)));
}


/***************************************************************
** NAME:        DsRelease
** SYNOPSIS:    void DsRelease(p);
**              CELP p;   cells to be release
** DESCRIPTION: Releases this cell and recursively all cells
**              chained to it.
** RETURNS:     void
***************************************************************/
void PASCAL DsRelease(p)
CELP p;
{
    if (ISNIL(p)) return;
    switch(TAGpart(p))
    {
    case TYPE_STR:
    case TYPE_VEC:
        DsFreeBuf(STRPpart(p));
	break;
#ifdef GCPORTS
    case TYPE_PRT:
        DsClosePort(CELPRT(q));         /* close before cell is released */
	break;
#endif
     default:
         if (_ISCAR(p)) DsRelease(CARpart(p));
         if (_ISCDR(p)) DsRelease(CDRpart(p));
     }
     DsFreeCell(p);
}


/***************************************************************
** NAME:        DsProtect
** SYNOPSIS:    void DsProtect(p);
**              CELP p;         cell to be protected
** DESCRIPTION: Protect this cell and recursively all cells
**              chained to it.
** RETURNS:     void
***************************************************************/
void PASCAL DsProtect(p)
CELP p;
{
    while (ISTRUE(p) && !MARKED(p))
    {
#ifdef GCPORTS
        if (TAGpart(p)==TYPE_PRT) CELPRT(p)->dir |= GCMARK; /* port is used */
#endif
        PROTECT(p);
        if (_ISCAR(p)) DsProtect(CARpart(p));
        if (!_ISCDR(p)) return;
        p=CDRpart(p);
    }
}


/***************************************************************
** NAME:        DsGarbageCollect
** SYNOPSIS:    DWORD DsGarbageCollect(p)
**              CELP p;         A cell to be protected.
** DESCRIPTION: Performs a garbage collect on the DScheme memory
**              All cells not used anymore, or accessible are
**              released. Port not used anymore are closed and
**              strings are released to the string space.
** RETURNS:     Number of released cells.
***************************************************************/
DWORD PASCAL DsGarbageCollect(cels)
CELP cels;
{
    CELP p, FAR *sp;
    int i;
    DWORD oldnum=GLOB(freecels);
    void (*oldsig)();

    DSGCmessage(GCstart);
    oldsig=signal(SIGINT, SIG_IGN);              /* Disable Control-C handler */
    DsMark(cels);                                            /* mark argument */
    DsMark(GLOB(curargs));
    DsMark(GLOB(curenv));                    /* mark all cells which are used */
    DsMark(GLOB(curexp));
    for (p=GLOB(freel); ISTRUE(p); p=CDRpart(p))
        SETMARK(p);                           /* mark all cells in free list. */
    for (i=0;i<GLOB(hashsize);i++)               /* mark all symbols as used! */
        DsMark(GLOB(hashtab[i]));
    for (sp=GLOB(sstack);sp<GLOB(stkptr);sp++)
        DsMark(*sp);                        /* mark all cells pushed on stack */
    DsCollect();                                /* Collect all unmarked cells */
    if ((GLOB(GCflag)&2) || DsFragmented()) 
        DsBufDefrag();                       /* Defragmatise the string space */
    DSGCmessage(GCstop);
    GLOB(GCflag)=FALSE;
    signal(SIGINT, oldsig);                       /* Enable Control-C handler */
    return(GLOB(freecels)-oldnum);
}


/***************************************************************
** NAME:        DsMark
** SYNOPSIS:    void DsMark(p);
**              CELP p;         cell to be DsMarked.
** DESCRIPTION: DsMarks this cell and recursively all cells
**              chained to it.
** RETURNS:     void
***************************************************************/
void PASCAL DsMark(p)
CELP p;
{
    for (; ISTRUE(p) && (!MARKED(p)); p=CDRpart(p))
    {
        SETMARK(p);
#ifdef GCPORTS
        if (TAGpart(p)==TYPE_PRT) CELPRT(p)->dir |= GCMARK; /* port is used */
#endif
        if (_ISCAR(p)) DsMark(CARpart(p));
        if (!_ISCDR(p)) return;
    }
}


/***************************************************************
** NAME:        DsCollect
** SYNOPSIS:    void DsCollect()
** DESCRIPTION: Collect all not DsMarked cells and releases them
**              to the free cell space. All DsMarked cells are
**              unDsMarked.
** RETURNS:     void
***************************************************************/
void PASCAL DsCollect()
{
    int i;
    CELP p,q;

    for (p=GLOB(fblk); ISTRUE(p); p=CDRpart(p))
    {
        DSGCmessage(GCrun);
        q=p;
        for (i=(int)CARIpart(p)-1;i>0;i--)
        {
            if (!MARKED(++q))                      /* Not marked=> release it */
            {
                switch (TAGpart(q))
                {
                case TYPE_VEC:
                case TYPE_STR:
		    DsFreeBuf(STRPpart(q));
                    break;
#ifdef GCPORTS
                case TYPE_PRT:
                    if (!(CELPRT(q)->dir&GCMARK)) /* close only it isn't used */
                        DsClosePort(CELPRT(q));/*close before cel is released */
#endif
                }
                DsFreeCell(q);
            }
        }
    }
    DSGCmessage(GCrun);
    for (p=GLOB(fblk); ISTRUE(p); p=CDRpart(p))
    {
        q=p;
        for (i=(int)CARIpart(p)-1;i>0;i--) UNMARK(++q);
    }
#ifdef GCPORTS
    for (i=GLOB(prtnum);i>0;i--) GLOB(ports)[i].dir &= ~GCMARK;
#endif
}
