/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHBIG.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/05/31
**
** DESCRIPTION: This module contains functions to handle the
**              Scheme numbers (except very large integers!)
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schnum.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHNUM.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHNUM.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:46:16   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"
#ifdef MSC
#include <float.h>
#endif

STATIC CELP BigOne;     /* This holds a big one */
STATIC CELP ZERO;      /* This holds a zero */

 /***************** Prototype(s) for SCHNUM.C **************************/

STATIC CELP PASCAL Ds_maxmin  __((int n, CELP p,int t));

STATIC USHORT PASCAL DsMakeCopy __((CELP arg));
STATIC CELP   PASCAL DsAddOne   __((CELP arg,int sign));

STATIC CELP CDECL Ds_max      __((int n, CELP args));
STATIC CELP CDECL Ds_min      __((int n, CELP args));
STATIC CELP CDECL Ds_plus     __((int n, CELP args));
STATIC CELP CDECL Ds_mult     __((int n, CELP args));
STATIC CELP CDECL Ds_minus    __((int n, CELP args));
STATIC CELP CDECL Ds_div      __((int n, CELP args));
STATIC CELP CDECL Ds_divide   __((CELP quo, CELP div));
STATIC CELP CDECL Ds_rem      __((CELP p, CELP q));
STATIC CELP CDECL Ds_mod      __((CELP p, CELP q));
STATIC CELP CDECL Ds_quotient __((CELP p, CELP q));
STATIC CELP CDECL Ds_is       __((CELP p, CELP q));
STATIC CELP CDECL Ds_le       __((CELP p, CELP q));
STATIC CELP CDECL Ds_lt       __((CELP p, CELP q));
STATIC CELP CDECL Ds_ge       __((CELP p, CELP q));
STATIC CELP CDECL Ds_gt       __((CELP p, CELP q));
STATIC CELP CDECL Ds_zero     __((CELP arg));
STATIC CELP CDECL Ds_posi     __((CELP arg));
STATIC CELP CDECL Ds_floor    __((CELP arg));
STATIC CELP CDECL Ds_round    __((CELP arg));
STATIC CELP CDECL Ds_nega     __((CELP arg));
STATIC CELP CDECL Ds_even     __((CELP arg));
STATIC CELP CDECL Ds_odd      __((CELP arg));
STATIC CELP CDECL Ds_abs      __((CELP arg));
STATIC CELP CDECL Ds_inc      __((CELP arg));
STATIC CELP CDECL Ds_dec      __((CELP arg));

STATIC EXTDEF math_ext[] =
    {
        {"+",               (EXTPROC)Ds_plus,     -1},
        {"-",               (EXTPROC)Ds_minus,    -1},
        {"*",               (EXTPROC)Ds_mult,     -1},
        {"/",               (EXTPROC)Ds_div,      -1},
        {"MAX",             (EXTPROC)Ds_max,      -1},
        {"MIN",             (EXTPROC)Ds_min,      -1},
        {"ZERO?",           (EXTPROC)Ds_zero,      1, 0},
        {"EVEN?",           (EXTPROC)Ds_even,      1, 0},
        {"ODD?",            (EXTPROC)Ds_odd,       1, 0},
        {"NEGATIVE?",       (EXTPROC)Ds_nega,      1, 0},
        {"POSITIVE?",       (EXTPROC)Ds_posi,      1, 0},
        {"FLOOR",           (EXTPROC)Ds_floor,     1, 0},
        {"ROUND",           (EXTPROC)Ds_round,     1, 0},
        {"ABS",             (EXTPROC)Ds_abs,       1, 0},
        {"INC",             (EXTPROC)Ds_inc,       1, 0},
        {"DEC",             (EXTPROC)Ds_dec,       1, 0},
        {"=",               (EXTPROC)Ds_is,        2, 0, 0},
        {"<",               (EXTPROC)Ds_lt,        2, 0, 0},
        {">",               (EXTPROC)Ds_gt,        2, 0, 0},
        {">=",              (EXTPROC)Ds_ge,        2, 0, 0},
        {"<=",              (EXTPROC)Ds_le,        2, 0, 0},
        {"QUOTIENT",        (EXTPROC)Ds_quotient,  2, 0, 0},
        {"REMAINDER",       (EXTPROC)Ds_rem,       2, 0, 0},
        {"DIVIDE",          (EXTPROC)Ds_divide,    2, 0, 0},
        ENDOFLIST
    };

#ifdef MATHTRAP
STATIC void CDECL DsMathError __((int sig,int fpe));
#endif

/***************************************************************
** NAME:        DSmath                                     [API]
** SYNOPSIS:    int DSmath(glo)
**              GLOBAL *glo;
** DESCRIPTION: This function initializes math system and links
**              it to the DScheme environment.
** RETURNS:     S_ERROR, if error occured.
**              S_OKAY otherwise.
***************************************************************/
int PASCAL DSmath(glo)
GLOBAL *glo;
{
    SETGLOB(glo,"DSmath");
    if (setjmp(GLOB(err_jmp)))   /* If error somewhere in extdef jump to here */
        return(DsRetCode());
    GLOB(bignum)=TRUE;
    DsFuncDef(math_ext);
    BigOne=item=DsGetCell(TYPE_BIGP);
    BIGpart(item)=1L;
    CDRpart(item)=NIL;
    DsDefVar(DsInsExt("*BIG-ONE*"));
    ZERO=item=DSINTCEL(0L);
    DsDefVar(DsInsExt("*ZERO*"));
#ifdef MATHTRAP
    if (signal(SIGFPE,DsMathError)==SIG_ERR)
        return(S_ERROR);
#endif
    return(S_OKAY);
}

#ifdef MATHTRAP
/**************************************************************
** NAME:        DsMathError
** SYNOPSIS:    STATIC void DsMathError(sig,fpe)
**              int sig,fpe;
** DESCRIPTION: Math error handle function. This function is
**              raised by the SIG_FPE signal.
** RETURNS:     void
**************************************************************/
STATIC void CDECL DsMathError(sig,fpe)
int sig,fpe;
{
    sig;
    switch(fpe)
    {
#ifdef MSC
    case FPE_ZERODIVIDE: DSERROR(ERRDIV0,item);
    case FPE_OVERFLOW:   DSERROR(ERROVRFLW,item);
#endif
    default:             DSERROR(ERRFLOAT,item);
    }
}
#endif
              
             
STATIC
USHORT PASCAL DsGetNTag(arg)
CELP arg;
{
    TESTNUM(arg);
    return(TAGpart(arg));
}


STATIC
USHORT PASCAL DsMakeCopy(arg)
CELP arg;
{
    USHORT tag;
    TESTNUM(arg);
    switch(tag=TAGpart(arg))
    {
    case TYPE_INT: item=DSINTCEL(INTpart(arg));break;
    case TYPE_FLT: item=DSFLTCEL(FLTpart(arg));break;
    default:       item=big_cpy(arg); break;
    }
    return tag;
}


STATIC
CELP PASCAL DsAddOne(arg,sign)
CELP arg;
int sign;
{
    TESTNUM(arg);
    switch(TAGpart(arg))
    {
    case TYPE_FLT: return DSFLTCEL(FLTpart(arg)+sign);
    case TYPE_INT: item = int2big(arg); break;
    default:       item = arg; break;
    }
    return big2int( sign==1 ? big_adds(item, BigOne)
                            : big_subs(item, BigOne));
}


/***************************************************************
** NAME:        DsCmpNumber
** SYNOPSIS:    CELP DsCmpNumber(p,q,typ)
**              CELP p,q;
**              int typ;
** DESCRIPTION: Compares the numbers.
**              t=0: test on equal
**              t=-1: test on less or equal
**              t=-2: test on less than
**              t=1: test on greater or equal
**              t=2: test on greator than
** RETURNS:     True when equal, false when different.
***************************************************************/
#define int_cmp(a1,a2)  ((a1<a2)?-1:(a1==a2)?0:1)

CELP PASCAL DsCmpNumber(p,q,typ)
CELP p,q;
int typ;
{
    USHORT tagp,tagq;

    tagp=DsGetNTag(p);
    tagq=DsGetNTag(q);
    if (tagp==TYPE_FLT || tagq==TYPE_FLT)
    {
        switch(typ)
        {
        case -2: RETBOO(makeflo2(p,tagp)< makeflo2(q,tagq));
        case -1: RETBOO(makeflo2(p,tagp)<=makeflo2(q,tagq));
        case  0: RETBOO(makeflo2(p,tagp)==makeflo2(q,tagq));
        case  1: RETBOO(makeflo2(p,tagp)>=makeflo2(q,tagq));
        case  2: RETBOO(makeflo2(p,tagp)> makeflo2(q,tagq));
        }
    }
    else
    {
        int val;
        if (tagp==TYPE_INT)
            val = (tagq==TYPE_INT)
                ? int_cmp(INTpart(p),INTpart(q))
                : big_cmp(makebig(p),q);
        else
            val = big_cmp(p,(tagq==TYPE_INT) ? makebig(q) : q);
        switch(typ)
        {
        case -2: RETBOO (val< 0);
        case -1: RETBOO (val<=0);
        case  0: RETBOO (val==0);
        case  1: RETBOO (val>=0);
        case  2: RETBOO (val> 0);
        }
    }
    return NIL;
}


/***************************************************************
** NAME:        Ds_maxmin
** SYNOPSIS:    CELP Ds_maxmin(n,q,t)
**              int n;          number of arguments
**              CELP q;         List of arguments
**              int t;          t=2, return min, max otherwise
** DESCRIPTION: Returns the smallest or biggest argument.
** RETURNS:     The smallest(t=2) of the biggest(t=-2).
***************************************************************/
STATIC CELP PASCAL Ds_maxmin(n,q,t)
int n,t;
CELP q;
{
    CELP res=CARpart(q);
    while (--n)                                 /* for each arg */
    {
        q=CDRpart(q);
        if (DsCmpNumber(res, (n==1)?q:CARpart(q), t)==Q_true)
            res=CARpart(q);
    }
    return(res);
}


/*-------------------- Scheme Functions --------------------*/

STATIC
CELP Ds_zero(arg)
CELP arg;
{
    switch(DsGetNTag(arg))
    {
    case TYPE_INT: RETBOO (INTpart(arg)==0L);
    case TYPE_FLT: RETBOO (FLTpart(arg)==0.0);
    default: RETBOO( (INTpart(arg)==0L) && (ISNIL(CDRpart(arg))) );
    }
}



STATIC
CELP Ds_posi(arg)
CELP arg;
{
    switch(DsGetNTag(arg))
    {
    case TYPE_INT: RETBOO (INTpart(arg)>=0L);
    case TYPE_FLT: RETBOO (FLTpart(arg)>=0.0);
    case TYPE_BIGP: return Q_true;
    default: return NIL;
    }
}


STATIC
CELP Ds_nega(arg)
CELP arg;
{
    switch(DsGetNTag(arg))
    {
    case TYPE_INT: RETBOO (INTpart(arg)<0L);
    case TYPE_FLT: RETBOO (FLTpart(arg)<=0.0);
    case TYPE_BIGN: return Q_true;
    default: return NIL;
    }
}


STATIC 
CELP Ds_floor(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_FLT)
        return DSINTCEL((long)floor(FLTpart(arg)));
    return arg;
}


STATIC 
CELP Ds_round(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_FLT)
        return DSINTCEL((long)floor(0.5+FLTpart(arg)));
    return arg;
}


STATIC
CELP Ds_rem(p,q)
CELP p,q;
{
    p=big_divs(makebig(p), makebig(q), &q);
    return q;
}


STATIC
CELP Ds_quotient(p,q)
CELP p,q;
{
    return big_divs(makebig(p), makebig(q), &q);
}

     
STATIC
CELP Ds_even(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_FLT) DSTERROR(arg);
    RETBOO (!(INTpart(arg)&1));
}


STATIC
CELP Ds_odd(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_FLT) DSTERROR(arg);
    RETBOO (INTpart(arg)&1);
}


STATIC
CELP Ds_is(p,q)
CELP p,q;
{
    return DsCmpNumber(p,q,0);
}


STATIC
CELP Ds_le(p,q)
CELP p,q;
{
    return DsCmpNumber(p,q,-1);
}


STATIC
CELP Ds_lt(p,q)
CELP p,q;
{
    return DsCmpNumber(p,q,-2);
}

STATIC
CELP Ds_ge(p,q)
CELP p,q;
{
    return DsCmpNumber(p,q,1);
}

STATIC
CELP Ds_gt(p,q)
CELP p,q;
{
    return DsCmpNumber(p,q,2);
}


STATIC
CELP Ds_abs(arg)
CELP arg;
{
    switch (DsMakeCopy(arg))
    {
    case TYPE_FLT: FLTpart(item)=fabs(FLTpart(item));break;
    case TYPE_INT: INTpart(item)=labs(INTpart(item));break;
    default:       TAGpart(item)=TYPE_BIGP;break;
    }
    return item;
}


STATIC
CELP Ds_dec(arg)
CELP arg;
{       
    return DsAddOne(arg,-1);
}


STATIC
CELP Ds_inc(arg)
CELP arg;
{
    return DsAddOne(arg,1);
}


/***************************************************************
** NAME:        Ds_plus
** SYNOPSIS:    CELP Ds_plus(n)
**              int n;          Number of arguments
** DESCRIPTION: Calculates the sum of all the list elements.
**              If all the list elements are integers, an
**              integer result is returned, otherwise an
**              floating point cel is returned.
** EXAMPLE:     (+ 2 3 4 5)                 = 14
**              (+ 4 76.5 6)                = 86.5
** RETURNS:     The sum of all the elements
***************************************************************/
STATIC
CELP Ds_plus(n,args)
int n;
CELP args;
{
    CELP q;
    USHORT res_typ;
    REAL fresult;

    if (n==0) return ZERO;
    if (n==1) return args;                    /* One argument: just return it */
    switch(res_typ=DsGetNTag(item=CARpart(args)))
    {
    case TYPE_FLT: fresult=FLTpart(item); break;
    case TYPE_INT: item=int2big(item); break;
    }
    while (--n)                                          /* for each arg left */
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);       
        if (res_typ==TYPE_FLT)
            fresult += makeflo2(q,DsGetNTag(q));
        else
        {
            switch(DsGetNTag(q))
            {
            case TYPE_FLT: fresult = big2real(item) + FLTpart(q);
                           res_typ = TYPE_FLT;break;
            case TYPE_INT: item = big_adds(item, int2big(q));break;
            default:       item = big_adds(item, q);break;
            }
        }
    }
    if (res_typ!=TYPE_FLT)
    	return big2int(item);
    return DSFLTCEL(fresult);
}


/***************************************************************
** NAME:        Ds_mult
** SYNOPSIS:    CELP Ds_mult(n)
**              int n;          Number of arguments
** DESCRIPTION: Calculates the product of all the list elements.
**              If all the list elements are integers, an
**              integer result is returned, otherwise an
**              floating point cel is returned.
** RETURNS:     The product of all the elements
***************************************************************/
STATIC
CELP Ds_mult(n,args)
int n;
CELP args;
{
    CELP   q;
    REAL   fresult;
    USHORT res_typ;

    if (n==0) return ZERO;
    if (n==1) return args;
    switch(res_typ=DsGetNTag(item=CARpart(args)))
    {
    case TYPE_FLT: fresult=FLTpart(item); break;
    case TYPE_INT: item=int2big(item); break;
    }
    while (--n)            /* for each arg left*/
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);       
        if (res_typ==TYPE_FLT)
            fresult *= makeflo2(q,DsGetNTag(q));
        else
        {
            switch(DsGetNTag(q))
            {
            case TYPE_FLT: fresult = big2real(item) * FLTpart(q);
                           res_typ = TYPE_FLT; break;
            case TYPE_INT: item = big_muls(item, int2big(q)); break;
            default:       item = big_muls(item, q); break;
            }
        }
    }
    if (res_typ!=TYPE_FLT)
    	return big2int(item);
    return DSFLTCEL(fresult);
}


/***************************************************************
** NAME:        Ds_minus
** SYNOPSIS:    CELP Ds_minus(n)
**              int n;          Number of arguments
** DESCRIPTION: Substracts the sum of the all but first elements
**              of the list from the first element. Except when
**              the element contains one element, then this
**              is negated and returned.
**              If all the list elements are integers, an
**              integer result is returned, otherwise an
**              floating point cel is returned.
** EXAMPLE:     (- 10 9)                             = 1
**              (- 1234.5)                           = -1234.5
** RETURNS:     The difference of all the elements.
***************************************************************/
STATIC
CELP Ds_minus(n,args)
int n;
CELP args;
{
    CELP   q;
    REAL   fresult;
    USHORT res_typ;

    if (n==0) return ZERO;
    if (n==1)
    {
        switch (DsMakeCopy(args))
        {
        case TYPE_FLT: FLTpart(item)=-FLTpart(item);break;
        case TYPE_INT: INTpart(item)=-INTpart(item);break;
        default: TAGpart(item) ^= TYPE_BIGN^TYPE_BIGP;break;
        }
	return item;
    }
    switch(res_typ=DsGetNTag(item=CARpart(args)))        /* first arg of more */
    {
    case TYPE_FLT: fresult=FLTpart(item); break;
    case TYPE_INT: item=int2big(item); break;
    }
    while (--n)
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);
        if (res_typ==TYPE_FLT)
            fresult -= makeflo2(q, DsGetNTag(q));
        else
        {
            switch(DsGetNTag(q))
            {
            case TYPE_FLT: res_typ = TYPE_FLT;
                           fresult = big2real(item) - FLTpart(q);break;
            case TYPE_INT: item = big_subs(item,int2big(q));break;
            default:       item = big_subs(item,q);break;
            }
        }
    }
    if (res_typ!=TYPE_FLT)
        return big2int(item);
    return DSFLTCEL(fresult);
}


/***************************************************************
** NAME:        Ds_div
** SYNOPSIS:    CELP Ds_div(n)
**              int n;          Number of arguments
** DESCRIPTION: Divides the arguments. (divide and conquer!)
** RETURNS:     The result.
***************************************************************/
STATIC
CELP Ds_div(n,args)
int n;
CELP args;
{
    CELP q,rem;
    USHORT res_typ;
    REAL fresult;

    if (n==0) return ZERO;
    if (n==1)
    {
        switch(DsGetNTag(args))
        {
        case TYPE_FLT: ZEROCHECK(args); return DSFLTCEL(1.0/FLTpart(args));
        case TYPE_INT: return big_divs(BigOne,int2big(args),&rem);
        default:       return big_divs(BigOne,args,&rem);
        }
    }
    switch(res_typ=DsGetNTag(item=CARpart(args)))
    {
    case TYPE_FLT: fresult=FLTpart(item); break;
    case TYPE_INT: item=int2big(item); break;
    }
    while (--n)
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);
        if (res_typ!=TYPE_FLT)
        {
            rem=NIL;
            switch(DsGetNTag(q))
            {
            case TYPE_FLT: res_typ=TYPE_FLT; ZEROCHECK(q);
                           fresult = big2real(item) / FLTpart(q);break;
            case TYPE_INT: item = big_divs(item,int2big(q),&rem);break;
            default:       item = big_divs(item,q,&rem);break;
            }
            if (rem) big_free(rem);
        }
        else
        {
            REAL tmp=makeflo2(q,DsGetNTag(q));
            ZEROCHECK(tmp);
            fresult /= tmp;
        }
    }
    if (res_typ!=TYPE_FLT)
        return big2int(item);
    return DSFLTCEL(fresult);
}


/***************************************************************
** NAME:        Ds_divide
** SYNOPSIS:    CELP Ds_divide(quo,div)
**              CELP quo,div;   dividend and divider.
** DESCRIPTION: Divides the first argument by the second.
**              And returns both quotient and remainder.
**              Works only on integers (long).
** EXAMPLE:     (/ 13 4)      = (3 . 1)
** RETURNS:     A pair with the quotient and the remainder
***************************************************************/
STATIC
CELP Ds_divide(a, b)
CELP a;
CELP b;
{
    a=big_divs(makebig(a), makebig(b), &b);
    return(DsCons(big2int(a),big2int(b)));
}


STATIC CELP Ds_max(n,p)
int n;
CELP p;
{
    return Ds_maxmin(n,p,-2);
}

STATIC CELP Ds_min(n,p)
int n;
CELP p;
{
    return Ds_maxmin(n,p,2);
}
