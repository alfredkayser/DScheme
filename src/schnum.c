/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHNUM.C
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
#ifdef __STDC__
#include <float.h>
#endif

#define FLOAT(p) (DsGetNTag(p)==TYPE_INT?(REAL)INTpart(p):FLTpart(p))

 /***************** Prototype(s) for SCHNUM.C **************************/

STATIC CELP   PASCAL Ds_maxmin  __((int n, CELP p,int t));
STATIC USHORT PASCAL DsGetNTag  __((CELP arg));

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
        {"NEGATIVE?",       (EXTPROC)Ds_nega,      1, 0},
        {"POSITIVE?",       (EXTPROC)Ds_posi,      1, 0},
        {"FLOOR",           (EXTPROC)Ds_floor,     1, 0},
        {"ROUND",           (EXTPROC)Ds_round,     1, 0},
        {"ABS",             (EXTPROC)Ds_abs,       1, 0},
        {"INC",             (EXTPROC)Ds_inc,       1, 0},
        {"DEC",             (EXTPROC)Ds_dec,       1, 0},
        {"ZERO?",           (EXTPROC)Ds_zero,      1, 0},
        {"EVEN?",           (EXTPROC)Ds_even,      1, TYPE_INT},
        {"ODD?",            (EXTPROC)Ds_odd,       1, TYPE_INT},
        {"=",               (EXTPROC)Ds_is,        2, 0, 0},
        {"<",               (EXTPROC)Ds_lt,        2, 0, 0},
        {">",               (EXTPROC)Ds_gt,        2, 0, 0},
        {">=",              (EXTPROC)Ds_ge,        2, 0, 0},
        {"<=",              (EXTPROC)Ds_le,        2, 0, 0},
        {"QUOTIENT",        (EXTPROC)Ds_quotient,  2, TYPE_INT, TYPE_INT},
        {"REMAINDER",       (EXTPROC)Ds_rem,       2, TYPE_INT, TYPE_INT},
        {"DIVIDE",          (EXTPROC)Ds_divide,    2, TYPE_INT, TYPE_INT},
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
    GLOB(bignum)=FALSE;
    DsFuncDef(math_ext);
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

STATIC
void CDECL DsMathError(sig,fpe)
int sig,fpe;
{
    sig;
    printf("MathError\n");
    switch(fpe)
    {
#ifdef UNIX
/* If FPE_INTDIV_TRAP, et al. are defined check them, */
/* generate general floating point error otherwise */
#ifdef FPE_INTDIV_TRAP
    case FPE_INTDIV_TRAP :
    case FPE_FLTDIV_TRAP : DSERROR(ERRDIV0,item);
    case FPE_FLTOVF_TRAP : DSERROR(ERROVRFLW,item);
    case FPE_FLTINEX_TRAP: break;   /* Ignore this one */
#endif 
#else
    case FPE_ZERODIVIDE  : DSERROR(ERRDIV0,item);
    case FPE_OVERFLOW    : DSERROR(ERROVRFLW,item);
#endif
    default: DSERROR(ERRFLOAT,item);
    }
}
#endif
              
             
STATIC
USHORT PASCAL DsGetNTag(arg)
CELP arg;
{
    TESTNUM(arg);
    if (_ISCDR(arg))
        DSVERROR(ERRNOBIG);       /* numbers with CDR's are bignumbers */
    return(TAGpart(arg));
}


STATIC
CELP Ds_zero(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_INT)
        RETBOO (INTpart(arg)==0L);
    else
        RETBOO (FLTpart(arg)==0.0);
}



STATIC
CELP Ds_posi(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_INT)
        RETBOO (INTpart(arg)>=0L);
    else
        RETBOO (FLTpart(arg)>=0.0);
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
CELP Ds_nega(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_INT)
        RETBOO (INTpart(arg)<0L);
    else
        RETBOO (FLTpart(arg)<0.0);
}


STATIC
CELP Ds_rem(p,q)
CELP p,q;
{
    return(CDRpart(Ds_divide(p,q)));
}


STATIC
CELP Ds_quotient(p,q)
CELP p,q;
{
    return(CARpart(Ds_divide(p,q)));
}

     
STATIC
CELP Ds_even(arg)
CELP arg;
{
    RETBOO (!(INTpart(arg)&1));
}

STATIC
CELP Ds_odd(arg)
CELP arg;
{
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
    if (DsGetNTag(arg)==TYPE_INT)
        return DSINTCEL(labs(INTpart(arg)));
    else
        return DSFLTCEL(fabs(FLTpart(arg)));
}

STATIC
CELP Ds_inc(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_INT)
        return DSINTCEL(INTpart(arg)+1);
    else
        return DSFLTCEL(FLTpart(arg)+1.0);
}

STATIC
CELP Ds_dec(arg)
CELP arg;
{
    if (DsGetNTag(arg)==TYPE_INT)
        return DSINTCEL(INTpart(arg)-1);
    else
        return DSFLTCEL(FLTpart(arg)-1.0);
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
    LONG result;
    REAL fresult;

    if (n==0) return DSINTCEL(0L);
    if (n==1) return args;
    q=CARpart(args); /* first of more args */
    if ((res_typ=DsGetNTag(q))==TYPE_INT)
        result =INTpart(q);
    else
        fresult=FLTpart(q); 
    while (--n)            /* for each arg left*/
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);
        if (res_typ==TYPE_INT)
        {
            if (DsGetNTag(q)==TYPE_INT)
                result += INTpart(q);
            else
            {
                res_typ=TYPE_FLT;
                fresult=(REAL)result + FLTpart(q);
            }
        }
        else
            fresult += FLOAT(q);
    }
    if (res_typ==TYPE_FLT)
        return DSFLTCEL(fresult);
    else
        return DSINTCEL(result);
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
    CELP q;
    int  res_typ;
    LONG result;
    REAL fresult;

    if (n==0) return DSINTCEL(0L);
    if (n==1) return args;
    q=CARpart(args);
    res_typ=DsGetNTag(q);
    if (res_typ==TYPE_INT)
        result =INTpart(q);
    else
        fresult=FLTpart(q); 
    while (--n)             /* for each arg left*/
    {
        args=CDRpart(args);
        q=(n==1)?args:CARpart(args);
        if (res_typ==TYPE_INT)
        {
            if (DsGetNTag(q)==TYPE_INT)
                result *= INTpart(q);
            else
            {
                res_typ=TYPE_FLT;
                fresult=(REAL)result * FLTpart(q);
            }
        }
        else
            fresult *= FLOAT(q);
    }
    if (res_typ==TYPE_FLT)
        return DSFLTCEL(fresult);
    else
        return DSINTCEL(result);
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
    CELP q;
    int res_typ;
    LONG result;
    REAL fresult;

    if (n>1)                /* more than one argument? => first is special */
    {
        q=CARpart(args);
        n--;
        res_typ=DsGetNTag(q);
        if (res_typ==TYPE_INT)
            result=INTpart(q);
        else
            fresult=FLTpart(q);   
    }
    else
    {
        result=0;
        res_typ=TYPE_INT;
    }
    while (n--)
    {
        args=CDRpart(args);
        q=n?CARpart(args):args; 
        if (res_typ==TYPE_INT)
        {
            if (DsGetNTag(q)==TYPE_INT)
                result -= INTpart(q);
            else
            {
                res_typ=TYPE_FLT;
                fresult=(REAL)result - FLTpart(q);
            }
        }
        else
            fresult -= FLOAT(q);
    }
    if (res_typ==TYPE_INT)
        return DSINTCEL(result);
    else
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
    CELP q;
    int res_typ;
    LONG result;
    REAL fresult;

    if (n>1)                /* (div 20 4) => 5 */
    {
        q=CARpart(args);
        n--;
        res_typ=DsGetNTag(q);
        if (res_typ==TYPE_INT)
            result=INTpart(q);
        else
            fresult=FLTpart(q);   
    }
    else
    {
        result=1;               /* (div)      => 1.0 */
        res_typ=TYPE_INT;       /* (div 2)    => 0.5 */
    }
    while (n--)
    {
        args=CDRpart(args);
        q=n?CARpart(args):args;
        if (DsGetNTag(q)==TYPE_INT)
        {
            if (res_typ==TYPE_INT)
            {
                if (INTpart(q)==0) DSERROR(ERRDIV0,item);
                if (result%INTpart(q)==0)                  /* no remainder ? */
                    result /= INTpart(q);
                else
                {
                    res_typ=TYPE_FLT;                    /* goto float var */
                    fresult=((REAL) result)/((REAL)INTpart(q));
                }
            }
            else
                fresult /= (REAL)INTpart(q);
        }
        else
        {
#ifndef MATHTRAP
            if (FLTpart(q)==0.0) DSVERROR(ERRDIV0); /* math lib will trap it */
#endif
            if (res_typ==TYPE_INT)
            {
                res_typ=TYPE_FLT;
                fresult=(REAL)result / FLTpart(q);
            }
            else
                fresult /= FLTpart(q);
        }
    }
    if (res_typ==TYPE_INT)
        return DSINTCEL(result);
    else
        return DSFLTCEL(fresult);
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
CELP PASCAL DsCmpNumber(p,q,typ)
CELP p,q;
int typ;
{
    REAL f1,f2;

    if (DsGetNTag(p)!=TYPE_INT)
    {
        f1=FLTpart(p);
        f2=FLOAT(q);
    }
    else
    {
        if (DsGetNTag(q)!=TYPE_INT)
        {
            f1=(REAL)INTpart(p);
            f2=FLTpart(q);
        }
        else
        {
            switch(typ)     /* both integers... */
            {
            case -2: RETBOO(INTpart(p)< INTpart(q));
            case -1: RETBOO(INTpart(p)<=INTpart(q));
            case  0: RETBOO(INTpart(p)==INTpart(q));
            case  1: RETBOO(INTpart(p)>=INTpart(q));
            case  2: RETBOO(INTpart(p)> INTpart(q));
            }
        }
    }
    switch(typ)     /* both floats...(or 1 float and 1 integer)... */
    {
    case -2: RETBOO(f1< f2);
    case -1: RETBOO(f1<=f2);
    case  0: RETBOO(f1==f2);
    case  1: RETBOO(f1>=f2);
    case  2: RETBOO(f1> f2);
    }
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
CELP Ds_divide(quo, div)
CELP quo;
CELP div;
{
    LONG q=INTpart(quo);
    LONG d=INTpart(div);
    return(DsCons(DSINTCEL(q/d),DSINTCEL(q%d)));
}


STATIC CELP Ds_max(n,p)
int n;
CELP p;
{
    return Ds_maxmin(n,p,0);
}

STATIC CELP Ds_min(n,p)
int n;
CELP p;
{
    return Ds_maxmin(n,p,1);
}

/***************************************************************
** NAME:        Ds_maxmin
** SYNOPSIS:    CELP Ds_maxmin(n,t)
**              int n;          number of arguments
**              int t;          t=1, return min, max otherwise
** DESCRIPTION: Returns the smallest or biggest argument.
** RETURNS:     The smallest(t=1) of the biggest(t!=1).
***************************************************************/
STATIC CELP PASCAL Ds_maxmin(n,q,t)
int n,t;
CELP q;
{
    CELP p,celres;

    n--;
    celres=n?CARpart(q):q;
    TESTNUM(celres);
    t=(t==1)?2:-2;
    while (n--)                                              /* for each arg */
    {
        q=CDRpart(q);
        p=n?CARpart(q):q;
        TESTNUM(p);
        if (DsCmpNumber(celres,p,t)==Q_true) celres=q;
    }
    return(celres);
}


/*--- BIG dummy's ---*/

CELP PASCAL DsStrBig(str)
char *str;
{
    str;
    DSVERROR(ERRNOBIG);
    return NULL;
}

char * PASCAL DsBigStr(p)
CELP p;
{
    p;
    DSVERROR(ERRNOBIG);
    return NULL;
}
