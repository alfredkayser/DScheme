/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHPRC.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/12/02
**                                                                  
** DESCRIPTION: Contains all the LISP functions as defined by
**              the SCHEME system.
**              The functions starting with Ds_ are kernel functions
**              These are essential functions of the SCHEME system.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schprc.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHPRC.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHPRC.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:46:10   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"
#include "schdef.h"


/***************************************************************
** NAME:        DsCompare
** SYNOPSIS:    CELP DsCompare(a1,a2)
**              CELP a1,a2;     The two arguments to DsCompare
** DESCRIPTION: Compares the values of two cel pointers.
** RETURNS:     NIL when they are different, Q_true otherwise
** SEE ALSO:    DsEq, DsEqv, DsEqual.
***************************************************************/
CELP PASCAL DsCompare(a1,a2)
CELP a1,a2;
{
    switch(TAG(a1))
    {
    case TYPE_NIL : RETBOO(ISNIL(a2)); 
    case TYPE_MAC :
    case TYPE_EXT : 
    case TYPE_SPC :
    case TYPE_KEY :
    case TYPE_SYM : RETBOO(a1==a2);
    case TYPE_BIGP:
    case TYPE_BIGN:
    case TYPE_INT :
    case TYPE_FLT : return DsCmpNumber(a1,a2,0);
    case TYPE_TMS : RETBOO(CELTIM(a1).time == CELTIM(a2).time &&
                           CELTIM(a1).date == CELTIM(a2).date &&
                           CELTIM(a1).fsec == CELTIM(a2).fsec);
    case TYPE_CHR : RETBOO(CELCHR(a1)==CELCHR(a2));
    case TYPE_PRT : RETBOO(CELPRT(a1)==CELPRT(a2));
    case TYPE_STR : RETBOO(strcmp(STRPpart(a1),STRPpart(a2))==0);
    case TYPE_OID : RETBOO(INTpart(a1)==INTpart(a2));
    case TYPE_VEC : return DsCmpVector(a1,a2);
    }
    DSVERROR(ERRUNT);
}


/***************************************************************
** NAME:        DsEqv
** SYNOPSIS:    CELP DsEqv(a1,a2)
**              CELP a1,a2;     The two arguments to DsCompare
** DESCRIPTION: Compares two cel pointers and when they are
**              different it DsCompares the values of the cells.
**              When a1 and a2 are pointers to pairs, the car
**              and cdr pointers are DsCompared.
** RETURNS:     NIL when they are different, Q_true otherwise
** SEE ALSO:    DsEq, DsEqual.
***************************************************************/
CELP PASCAL DsEqv(a1,a2)
CELP a1,a2;
{
    if (a1==a2) return(Q_true);  /* ok */

    if (ISFALS(a1) || ISFALS(a2)) return(Q_false); /* not ok */
    if (TAGpart(a1)!=TAGpart(a2)) return(Q_false);
    if (_ISCAR(a1))
        RETBOO(CARpart(a1)==CARpart(a2) && CDRpart(a1)==CDRpart(a2));
    else
        return(DsCompare(a1,a2));
}


/***************************************************************
** NAME:        DsEqual
** SYNOPSIS:    CELP DsEqual(a1,a2)
**              CELP a1,a2;     The two arguments to DsCompare
** DESCRIPTION: Compares two cel pointers and when they are
**              different it DsCompares the values of the cells.
**              When a1 and a2 are pointers to pairs, the car
**              and cdr are tested with DsEqual (recursively!).
** RETURNS:     NIL when they are different, Q_true otherwise
** SEE ALSO:    DsEqv, DsEq.
***************************************************************/
CELP PASCAL DsEqual(a1,a2)
CELP a1,a2;
{
    if (a1==a2) return(Q_true);  /* ok */
    if (ISFALS(a1) || ISFALS(a2)) return(Q_false); /* not ok */
    if (TAGpart(a1)!=TAGpart(a2)) return(Q_false);
    if (_ISCAR(a1))
         RETBOO( ISTRUE(DsEqual(CARpart(a1),CARpart(a2))) &&
                 ISTRUE(DsEqual(CDRpart(a1),CDRpart(a2))) );
    else
        return(DsCompare(a1,a2));
}


/***************************************************************
** NAME:        DsCmpTime
** SYNOPSIS:    CELP DsCmpTime(a1,a2,t)
**              CELP a1,a2;     The two timestamps to DsCompare
**              int  t;         type test;
** DESCRIPTION: Compares two timestamps
**              Timestamp is an extention to the standard Scheme
**              t=0: test on DsEqual
**              t=1: test on greater or DsEqual
**              t=2: test on greator than
** RETURNS:     NIL when they are different, Q_true otherwise
***************************************************************/
CELP PASCAL DsCmpTime(a1,a2,t)
CELP a1,a2;
int t;
{
    TYPCHECK(a1,TYPE_TMS);
    TYPCHECK(a2,TYPE_TMS);
    if (t==0)   
        RETBOO(CELTIM(a1).time == CELTIM(a2).time &&
               CELTIM(a1).date == CELTIM(a2).date &&
               CELTIM(a1).fsec == CELTIM(a2).fsec);
    else
    {
        if (CELTIM(a1).date!=CELTIM(a2).date)
            RETBOO(CELTIM(a1).date > CELTIM(a2).date);      /* date is bigger */
        if (CELTIM(a1).time!=CELTIM(a2).time)
            RETBOO(CELTIM(a1).time > CELTIM(a2).time);      /* time is bigger */
        if (t==1)                                            /* less or equal */
            RETBOO(CELTIM(a1).fsec >= CELTIM(a2).fsec);
        else                                                     /* less than */
            RETBOO(CELTIM(a1).fsec > CELTIM(a2).fsec);
    }
}


/***************************************************************
** NAME:        DsCmpChar
** SYNOPSIS:    CELP DsCmpChar(a1,a2,t)
**              CELP a1,a2;     The two chars to compare
**              int  t;         type test;
** DESCRIPTION: Compares two characters.
**              t=0: test on equal
**              t=1: test on greater or equal
**              t=2: test on greator than
** RETURNS:     NIL when they are different, Q_true otherwise
***************************************************************/
CELP PASCAL DsCmpChar(a1,a2,t)
CELP a1,a2;
int t;
{
    TYPCHECK(a1,TYPE_CHR);
    TYPCHECK(a2,TYPE_CHR);
    switch(t)
    {
    case 0: RETBOO(CHRpart(a1) == CHRpart(a2));
    case 1: RETBOO(CHRpart(a1) >= CHRpart(a2));
    case 2: RETBOO(CHRpart(a1) > CHRpart(a2));
    case 3: RETBOO(tolower(CHRpart(a1)) == tolower(CHRpart(a2)));
    case 4: RETBOO(tolower(CHRpart(a1)) >= tolower(CHRpart(a2)));
    case 5: RETBOO(tolower(CHRpart(a1)) >  tolower(CHRpart(a2)));
    }
    return NIL;
}


/***************************************************************
** NAME:        DsAssQ
** SYNOPSIS:    void DsAssQ(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Finds item in list whose car is same (DsEq)
**              as key and returns it.
** RETURNS:     NIL when item is not found, item otherwise.
** SEE ALSO:    DsEq
***************************************************************/
CELP PASCAL DsAssQ(key,list)
CELP key,list;     
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (key==DsCaar(list))
            return CARpart(list);
    return NIL;
}


/***************************************************************
** NAME:        DsAssV
** SYNOPSIS:    void DsAssV(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Finds item in list whose car is same (DsEqv)
**              as key and returns it.
** RETURNS:     NILL when item is not found, item otherwise.
** SEE ALSO:    DsEqv
***************************************************************/
CELP PASCAL DsAssV(key,list)
CELP key,list;
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (ISTRUE(DsEqv(key,DsCaar(list))))
            return CARpart(list);
    return Q_false;
}


/***************************************************************
** NAME:        DsAssoc
** SYNOPSIS:    void DsAssoc(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Finds item in list whose car is same (DsEqual)
**              as key and returns it.
** RETURNS:     Q_false when item is not found, item otherwise.
** SEE ALSO:    DsEqual
***************************************************************/
CELP PASCAL DsAssoc(key,list)
CELP key,list;
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (ISTRUE(DsEqual(key,DsCaar(list))))
            return CARpart(list);
    return Q_false;
}


/***************************************************************
** NAME:        DsMemQ
** SYNOPSIS:    CELP DsMemQ(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Tests if key is somewhere in the list.
** RETURNS:     Q_false when item is not found.
**              pointer to list starting with key otherwise.
** SEE ALSO:    DsEq
***************************************************************/
CELP PASCAL DsMemQ(key,list)
CELP key,list;
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (key==DsCar(list))
            return list;
    return Q_false;
}


/***************************************************************
** NAME:        DsMemV
** SYNOPSIS:    CELP DsMemV(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Tests if key is somewhere in the list.
** RETURNS:     Q_false when item is not found.
**              pointer to list starting with key otherwise.
** SEE ALSO:    DsEqv
***************************************************************/
#undef  FUNCTION
#define FUNCTION "DsMemV"

CELP PASCAL DsMemV(key,list)
CELP key,list;
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (ISTRUE(DsEqv(key,DsCar(list))))
            return list;
    return Q_false;
}


/***************************************************************
** NAME:        DsMember
** SYNOPSIS:    CELP DsMember(key,list)
**              CELP key;       Key to search for
**              CELP list;      List to search in
** DESCRIPTION: Tests if key is somewhere in the list.
** RETURNS:     Q_false when item is not found.
**              pointer to list starting with key otherwise.
** SEE ALSO:    DsEqual
***************************************************************/
CELP PASCAL DsMember(key,list)
CELP key,list;
{
    for (;ISTRUE(list);list=CDRpart(list))
        if (ISTRUE(DsEqual(key,DsCar(list)))) 
            return list;
    return Q_false;
}
     

/***************************************************************
** NAME:        DsLength
** SYNOPSIS:    LONG DsLength(list)
**              CELP list;      List
** DESCRIPTION: Calculates the length of the list
** RETURNS:     The length.
***************************************************************/
LONG PASCAL DsLength(l)
CELP l;
{
    register LONG t=0;
    for (;ISCDR(l);l=CDRpart(l)) t++; /* count each element of the list */
    return(t);
}


/***************************************************************
** NAME:        Ds_load
** SYNOPSIS:    CELP Ds_load(l)
**              CELP l;         Pointer to cell with filename
** DESCRIPTION: Opens the file (on S.stack) in READMODE and
**              redirects inport to the new port until the EOF
**              is found. This results into the 'loading' of
**              a file of scheme commands.
** RETURNS:     TRUE, when open succeeds.
***************************************************************/
CELP Ds_load(name)
CELP name;
{
    PORT *newp;
    CELP exp;

    if ((newp=DsFOpen(STRPpart(name),READMODE))==NIL)
        return(Q_false);                                   /* file not found! */
    while ((exp=DsRead(newp))!=Q_eof)
        DsEval(exp);
    DsClosePort(newp);                                     /* end of old port */
    return(Q_invis);                                        /* end of loading */
}


/***************************************************************
** NAME:        Ds_math1
** SYNOPSIS:    CELP Ds_math1(opcode,arg)
**              int opcode;     Nr of operation
**              CELP arg;       Argument
** DESCRIPTION: Performs all unary operations.
**              It switches first on argument type, and performs
**              then the operation.
** RETURNS:     the answer.
***************************************************************/
CELP PASCAL Ds_math1(opcode,arg)
int opcode;
CELP arg;
{
    if (ISNIL(arg))
        RETBOO(opcode==IP_ISBOOL);
    if (opcode==IP_ISATOM && TAGpart(arg)!=TYPE_PAIR)
        return Q_true;
    switch(TAGpart(arg))
    {
    case TYPE_INT:
        if ( opcode==IP_ISINT
            || opcode==IP_ISNUMBER) return Q_true;
        break;

    case TYPE_FLT:
        if ( opcode==IP_ISREAL
            || opcode==IP_ISNUMBER) return Q_true;
        break;

    case TYPE_TMS:
        if (opcode==IP_ISTIME) return Q_true;
        break;

    case TYPE_PAIR:
        switch(opcode)
        {
        case IP_ISLIST  :if ISPAIR(CDRpart(arg)) return Q_true;
        case IP_ISPAIR  :return Q_true;
        }
        break;

    case TYPE_PRT:
        switch(opcode)
        {
        case IP_ISIPORT :if (CELPRT(arg)->dir & READMODE) return Q_true;
        case IP_ISOPORT :if (CELPRT(arg)->dir & WRITMODE) return Q_true;
        case IP_ISPORT  :return Q_true;
        }
        break;

    case TYPE_CHR:
        switch(opcode)
        {
        case IP_ISCHAR:    return Q_true;
        case IP_ISCHALPHA: if (isalpha(CHRpart(arg))) return Q_true;break;
        case IP_ISCHNUMER: if (isdigit(CHRpart(arg))) return Q_true;break;
        case IP_ISCHWHITE: if (isspace(CHRpart(arg))) return Q_true;break;
        case IP_ISCHUPERR: if (isupper(CHRpart(arg))) return Q_true;break;
        case IP_ISCHLOWER: if (islower(CHRpart(arg))) return Q_true;break;
        }
        break;

    case TYPE_STR:
        if (opcode==IP_ISSTR) return Q_true;        
        break;

    case TYPE_BIGN:
    case TYPE_BIGP:
        if  ( opcode==IP_ISBIGNUM
            || opcode==IP_ISINT
            || opcode==IP_ISNUMBER) return Q_true;
        break;

    case TYPE_PRC:
        if (opcode==IP_ISPROC) return Q_true;
        break;

    case TYPE_SYM:
        if (opcode==IP_ISSYM) return Q_true;
        break;

    case TYPE_VEC:
        if (opcode==IP_ISVECTOR) return Q_true;
        break;

    default:
        switch(opcode)
        {
        case IP_ISBOOL:if (arg==Q_true) return Q_true;
        case IP_ISEOF :if (arg==Q_eof) return Q_true;
        }
    }
    return Q_false;
}


