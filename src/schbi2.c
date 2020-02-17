/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHBI2.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/05/29
**
** DESCRIPTION: This module contains functions to handle the
**              Scheme Bignumbers (very large integers!)
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schbig.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHBIG.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHBIG.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:45:42   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"

                       /* 0  1   2    3     4      5       6        7        */
static ULONG bigfactor[]={1L,10L,100L,1000L,10000L,100000L,1000000L,10000000L};
static REAL  ffactor[]={1e8,1e16};                                   
#ifdef MSC
STATIC ULONG inprod __((USHORT,USHORT,USHORT,USHORT));
#else
#define inprod(a,b,c,d) (((ULONG)a*(ULONG)b)+((ULONG)c*(ULONG)d))
#endif


/***************************************************************
** NAME:        DsStrBig
** SYNOPSIS:    CELP DsStrBig(str)
**              char *str
** DESCRIPTION: Bigcel converts a string with digits into a
**              Scheme bignum.
** RETURNS:     A pointer to a Scheme bignum.
***************************************************************/
CELP PASCAL DsStrBig(str)
char *str;                  
{                           
    CELP fp,cp;
    int  i,j;
    char *p;

    p=str;
    if (p[0]=='-') p++;                                        /* skip - sign */
    while (*p=='0') p++;                                       /* skip zero's */
    if (*p=='\0')                                  /* empty string means zero */
        return DsGetCell(TYPE_BIGP);       /* DsGetCell ensures value is zero */
    for (i=0;*p;p++,i++)                            /* count number of digits */
        if (!isdigit(*p))                     /* check if they are all digits */
            DsStrError(ERRCHARN,str);
    cp=fp=DsGetCell(TYPE_BIGP);
    while (1)                           /* work from Least sign.to Most Sign. */
    {
        p -= (j=min(i,8));                    /* size of part to be converted */
        BIGpart(cp) = atol(p);
        if ((i-=j)==0) break;
        cp = CDRpart(cp) = DsGetCell(TYPE_BIGP);             /* get next cell */
        *p='\0';                                   /* mark parts already done */
    }
    if (*str=='-') TAGpart(fp)=TYPE_BIGN;                  /* negative number */
    return(fp);
}


/***************************************************************
** NAME:        DsBigStr
** SYNOPSIS:    char *DsBigStr(p)
**              CELP p;
** DESCRIPTION: Celbig converts a bignum to a string with digits
** RETURNS:     A pointer to a string (in BIGBUF)
***************************************************************/
char * PASCAL DsBigStr(p)
CELP p;
{                                           /* "1 23456789 01234567 89012345" */
    SHORT  sign,l;
    char   *q;                       /* 89012345 -> 01234567 -> 23456789 -> 1 */
    USHORT t;
    ULONG  ll;

    l=(SHORT)DsLength(p)*8;                                /* number of cells */
    if (l>=BIGMAX)                            /* test on maximum bufferlength */
        DSVERROR(ERRBIGBIG);           /* can't pass argument to errorhandler */
    q=BIGBUF+l+1;                                  /* inclusive space for '-' */
    q[0]='\0';
    sign = 1;
    if (TAGpart(p)==TYPE_BIGN) sign=-1;
    for (;l;l-=8)
    {
        ll=BIGpart(p);                              /* get value of this part */
        p=CDRpart(p);                                  /* next part of bignum */
        t=(USHORT)(ll % 10000L);                         /* first four digits */
        *--q=(char)((t % 10)+'0');t/=10; 
        *--q=(char)((t % 10)+'0');t/=10; 
        *--q=(char)((t % 10)+'0'); 
        *--q=(char)((t / 10)+'0');  
        t=(USHORT)(ll / 10000L);                        /* last four digits */  
        *--q=(char)((t % 10)+'0');t/=10;  
        *--q=(char)((t % 10)+'0');t/=10;  
        *--q=(char)((t % 10)+'0');        
        *--q=(char)((t / 10)+'0');        
    }
    while (*q=='0') q++;                            /* discard leading zero's */
    if (*q=='\0')
        *(--q)='0';                    /* if string is empty than number is 0 */
    else
        if (sign<0)
            *(--q)='-';                                  /* add negative sign */
    return(q);
}


/***************************************************************
** NAME:        big_cpy
** SYNOPSIS:    CELP big_cpy(p)
**              CELP p;
** DESCRIPTION: Big_cpy copies a bignum in a newly allocated
**              linked list of cells.
** RETURNS:     A new list with the same BIGnumber.
***************************************************************/
CELP PASCAL big_cpy(num)
CELP num;
{
    CELP fp,cp;

    if (ISNIL(num)) return NIL;
    cp=fp=DsGetCell(TAGpart(num));
    while (1)
    {
        BIGpart(cp)=BIGpart(num);                            /* copy contents */
        if (ISNIL(num=CDRpart(num))) break;              /* no more ? => quit */
        cp=CDRpart(cp)=DsGetCell(TYPE_BIGP);             /* allocate new cell */
    }
    return(fp);
}


/***************************************************************
** NAME:        big_add
** SYNOPSIS:    CELP big_add(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_add adds two bignumber together and
**              produces a new bignum.
** RETURNS:     The sum of n1 and n2.
***************************************************************/
CELP PASCAL big_add(n1,n2)
CELP n1,n2;
{
    CELP fp,cp;
    USHORT carry;

    fp=cp=DsGetCell(TYPE_BIGP);
    while(1)
    {
        if (ISTRUE(n1))
        {
            BIGpart(cp)+=BIGpart(n1);
            n1=CDRpart(n1);
        }
        if (ISTRUE(n2))
        {
            BIGpart(cp)+=BIGpart(n2);
            n2=CDRpart(n2);
        }
        if (carry=BIGpart(cp)>=BIGBASE)
            BIGpart(cp)-=BIGBASE;
        if (ISNIL(n1) && ISNIL(n2) && !carry) break;  /* nothing left */
        cp=CDRpart(cp)=DsGetCell(TYPE_BIGP);          /* next cell */
        BIGpart(cp)=(ULONG)carry;                     /* store carry */
    }
    return(fp);
}


/***************************************************************
** NAME:        big_sub
** SYNOPSIS:    CELP big_sub(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_sub subtracts two bignumbers and
**              produces a new bignum.
** RETURNS:     (- n1 n2)
***************************************************************/
CELP PASCAL big_sub(n1,n2)
CELP n1,n2;
{
    CELP fp,cp,lp;
    LONG tmp=0L;

    cp=lp=fp=DsGetCell(TYPE_BIGP);
    while (1)
    {
        if (ISTRUE(n1))
        {
            tmp+=BIGpart(n1);
            n1=CDRpart(n1);
        }
        if (ISTRUE(n2))
        {
            tmp-=BIGpart(n2);
            n2=CDRpart(n2);
        }
        BIGpart(cp)=(tmp<0)?tmp+BIGBASE:tmp;         /* borrow from next part */
        if (BIGpart(cp)) lp=cp;       /* most significant part with no zero's */
        tmp=(tmp<0L)?-1L:0;
        if (ISNIL(n1) && ISNIL(n2)) break;     /* no more digits to substract */
        cp=CDRpart(cp)=DsGetCell(TYPE_BIGP);                 /* Get next part */
    }
    if (tmp)                                          /* result is negative ! */
    {
        cp=lp=fp;
        TAGpart(cp)=TYPE_BIGN;
        BIGpart(cp)=BIGBASE-BIGpart(cp);
        while (ISTRUE(cp=CDRpart(cp)))
            if (BIGpart(cp)=(BIGBASE-1)-BIGpart(cp))   /* take (base-1) cmpl. */
                lp=cp;           /* find most significant part with no zero's */
    }
    CDRpart(lp)=NIL;            /* all parts after lp are zero, wipe them */
    return(fp);
}


/***************************************************************
** NAME:        big_muli
** SYNOPSIS:    CELP big_muli(ln,n2)
**              LONG  ln;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_muli multiplies a bignum with a long
**              integer. This is a subfunction of bug_mul.
** RETURNS:     The product of ln and n2.
***************************************************************/
#ifdef MSC
STATIC ULONG inprod(a,b,c,d) 
int a,b,c,d;
{
    return ((ULONG)a*(ULONG)b)+((ULONG)c*(ULONG)d);
}
#endif

CELP PASCAL big_muli(l,num)
ULONG l;
CELP num;
{
    CELP fp,cp;
    USHORT Al,Ah,Bl,Bh;
    ULONG t,overflow=0;

    Al=(USHORT)(l%10000L);
    Ah=(USHORT)(l/10000L);
    fp=cp=DsGetCell(TYPE_BIGP);
    while (ISTRUE(num))
    {                             
        Bl=(USHORT)(BIGpart(num)%10000L);
        Bh=(USHORT)(BIGpart(num)/10000L);
        num=CDRpart(num);
        t=inprod(Ah, Bl, Al, Bh);
        BIGpart(cp)=overflow+inprod(Al, Bl, 10000, (USHORT)(t%10000L));
        overflow=inprod(Ah, Bh, 1, (USHORT)(t/10000L));
        if (BIGpart(cp)>=BIGBASE)
        {                 
            BIGpart(cp)-=BIGBASE;
            overflow++;
        }
        if (ISNIL(num) && overflow==0) break;
        cp=CDRpart(cp)=DsGetCell(TYPE_BIGP);
    }
    if (overflow) BIGpart(cp)=overflow;
    return(fp);
}


/***************************************************************
** NAME:        big_free
** SYNOPSIS:    void big_free(p)
**              CELP p;         A bignumber (or linked list)
** DESCRIPTION: big_free releases a bignumber to the free cell
**              space.
** RETURNS:     void
***************************************************************/
void PASCAL big_free(p)
CELP p;
{
    CELP next;
    while (ISTRUE(p))
    {
        next=CDRpart(p);                                /* point to next cell */
        DsFreeCell(p);                                        /* release cell */
        p=next;
    }
}


/***************************************************************
** NAME:        big_mul
** SYNOPSIS:    CELP big_mul(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_mul multiplies number1 with number2.
** RETURNS:     The product of n1 and n2.
***************************************************************/
CELP PASCAL big_mul(n1,n2)
CELP n1,n2;
{
    CELP pp,rp,sump,zerop1,zerop2;

    sump=big_muli(BIGpart(n1),n2);                               /* first hit */
    if (n1=CDRpart(n1))
    {
        zerop1=zerop2=DsGetCell(TYPE_BIGP);
        while (1)
        {
            pp=big_muli(BIGpart(n1),n2);             /* calculate temp result */
            CDRpart(zerop2)=pp;                   /* chain a few zero's to it */
            n1=CDRpart(n1);
            rp=big_add(sump,zerop1);               /* add this to sum pointer */
            CDRpart(zerop2)=NIL;           /* rest of chain is to be released */
            big_free(pp);                              /* release temp result */
            big_free(sump);                         /* release old sum result */
            sump=rp;                                   /* point to new result */
            if (ISNIL(n1)) break;                    /* end of the multiplyer */
            zerop2=CDRpart(zerop2)=DsGetCell(TYPE_BIGP);  /* chain extra zero */
        } 
        big_free(zerop1);                               /* release all zero's */
    }
    return(sump);
}


/***************************************************************
** NAME:        big_len
** SYNOPSIS:    CELP big_len(number)
**              CELP number;    a bignumber
** DESCRIPTION: big_len calculates the number of digits in a
**              bignumber.
** RETURNS:     The length of number.
***************************************************************/
int PASCAL big_len(p)
CELP p;
{
    int l,i;
    ULONG t;

    for (l=0; ISTRUE(CDRpart(p)); p=CDRpart(p)) l++;          /* count parts */
    for (t=BIGpart(p),i=0;i<8 && t>=bigfactor[i];i++);  /* cnt digits in last */
    return((l<<3)+i);                            /* 8 digits per part  \\ part */
}


/***************************************************************
** NAME:        big_mul10
** SYNOPSIS:    CELP big_mul10(n,n2)
**              int  n;         a power of 10.
**              CELP n2;        Number2
** DESCRIPTION: big_mul10 multiplies a bignum with the n-th
**              power of 10. This is a subfunction of big_div.
**              Instead of multipling with 10.00000000.00000000
**              will number2 be multiplied by 10 and 16 digits
**              are chained at the end.
** RETURNS:     The product of 10^n and n2.
***************************************************************/
CELP PASCAL big_mul10(l,num)
int l;
CELP num;
{
    CELP tp;

    num = (l&0x7) ? big_muli(bigfactor[l&0x7],num) : big_cpy(num);
    l>>=3;                             /* res = 10.00000000.00000000.00000000 */
    while (l--)                                /* preprend 3 times a 00000000 */
    {
        tp=DsGetCell(TYPE_BIGP);                            /* Get a new cell */
        CDRpart(tp)=num;                              /* chain in front of it */
        num=tp;
    }
    return(num);
}


/***************************************************************
** NAME:        big_div
** SYNOPSIS:    CELP big_div(dd,div,prem)
**              CELP dd;        Dividend
**              CELP div;       Divider
**              CELP *prem;     Pointer to remainder
** DESCRIPTION: big_div divides dd by div and returnes the
**              quotient and the remainder is returned via the
**              prem pointer.
** RETURNS:     The quotient
***************************************************************/
CELP PASCAL big_div(dd,div,prem)
CELP dd,div;
CELP *prem;
{
    CELP rp,dp,rem;
    int lquo,ldiv;
    char *p,c;

    if (ISNIL(CDRpart(div)) && BIGpart(div)==0L) DSERROR(ERRDIV0,DsCons(dd,div));
    p=BIGBUF;                                       /* point to result buffer */
    rem=dd;            /* start condition:  QUOtient=DIVider*RESult+REMainder */
    lquo=big_len(dd);
    ldiv=big_len(div);
    if (lquo-ldiv>=BIGMAX)
        DSERROR(ERRBIGBIG,DsCons(dd,div));  /* result wil become much too big */
    while (lquo>=ldiv)
    {
        dp=big_mul10(lquo-ldiv,div);                 /* div *= 10^(lquo-ldiv) */
        c='0';                                             /* start with zero */
        while (1)
        {
            rp=big_sub(rem,dp);                          /* substract divider */
            if (TAGpart(rp)==TYPE_BIGN)               /* it became negative ? */
                break;                                                /* oops */
            c++;
            rem=rp;                                    /* store new remainder */
        }
        lquo--;                                 /* number of digits decreased */
        *p++=c;                                               /* store result */
        big_free(dp);                                 /* release temp divider */
    }
    *p='\0';                                                 /* end of string */
    *prem=rem;                                /* return remainder via pointer */
    return(DsStrBig(BIGBUF));
}
                                         

/***************************************************************
** NAME:        big_cmp
** SYNOPSIS:    CELP big_cmp(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_cmp compares number1 with number2.
**              It actually performs a subtraction without
**              keeping the result.
** RETURNS:     -1 when n1<n2, 0 when n1=2n, and 1 when n1>n2.
***************************************************************/
int PASCAL big_cmp(n1,n2)
CELP n1,n2;
{
    LONG tmp=0L, ored=0L;

    while (ISTRUE(n1) || ISTRUE(n2))
    {
        if (ISTRUE(n1))
        {
            tmp+=BIGpart(n1);
            n1=CDRpart(n1);
        }
        if (ISTRUE(n2))
        {
            tmp-=BIGpart(n2);
            n2=CDRpart(n2);
        }
        ored |= tmp;                    /* ored is 0 when all digits are zero */
        tmp = (tmp<0L) ? -1L : 0L;                   /* borrow from next part */
    }
    return tmp ? -1 : (ored==0) ? 0 : 1;
}


/***************************************************************
** NAME:        big_adds
** SYNOPSIS:    CELP big_adds(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_adds adds two numbers, but checks the signs
**              first.
** RETURNS:     The sum of n1 and n2.
***************************************************************/
CELP PASCAL big_adds(n1,n2)
CELP n1,n2;
{
    if (TAGpart(n1)==TYPE_BIGP)         /* number 1 positive ? */
        return (TAGpart(n2)==TYPE_BIGP) /* number 2 also positive ? */
               ? big_add(n1,n2)         /* n1+n2 */
               : big_sub(n1,n2);        /* calc n1-n2  instead of n1+-n2 */
    else
    {                                  /* number 1 is negative */
        if (TAGpart(n2)==TYPE_BIGP)    /* number 2 positive ? */
            return big_sub(n2,n1);     /* calc n2-n1 instead of -n1+n2 */
        else
        {
            CELP res;
            res = big_add(n1,n2);      /* calc -(n1+n2)  instead of -n1+-n2 */
            TAGpart(res) = TYPE_BIGN;
            return res;
        }
    }
}


/***************************************************************
** NAME:        big_subs
** SYNOPSIS:    CELP big_subs(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_subs adds two numbers, but checks the signs
**              first.
** RETURNS:     The sum of n1 and n2.
***************************************************************/
CELP PASCAL big_subs(n1,n2)
CELP n1,n2;
{
    if (TAGpart(n1)==TYPE_BIGP)         /* number 1 positive ? */
        return (TAGpart(n2)==TYPE_BIGP) /* number 2 also positive ? */
               ? big_sub(n1,n2)         /* n1-n2 */
               : big_add(n1,n2);        /* calc n1+n2  instead of n1--n2 */
    else
    {
        if (TAGpart(n2)==TYPE_BIGP)     /* positive ? */
        {
            CELP res;
            res=big_add(n1,n2);         /* perform -(n1+n2) instead of -n1-n2 */
            TAGpart(res)=TYPE_BIGN;     /* make result negative */
            return(res);
        }
        else
            return big_sub(n2,n1);      /* perform n2-n1  instead of -n1--n2 */
    }
}


/***************************************************************
** NAME:        big_muls
** SYNOPSIS:    CELP big_muls(n1,n2)
**              CELP n1;        Number 1
**              CELP n2;        Number 2
** DESCRIPTION: big_muls adds two numbers, but checks the signs.
** RETURNS:     The sum of n1 and n2.
***************************************************************/
CELP PASCAL big_muls(n1,n2)
CELP n1,n2;
{
    CELP res = big_mul(n1,n2);
    if (TAGpart(n1)!=TAGpart(n2))      /* Unequal signs so result is negative */
        TAGpart(res)=TYPE_BIGN;
    return res;
}


/***************************************************************
** NAME:        big_divs
** SYNOPSIS:    CELP PASCAL big_divs(quo, div, prem)  
**              CELP quo,div;                         
**              CELP *prem;                           
** DESCRIPTION: big_divs adds two numbers, but checks the signs
**              first.
** RETURNS:     the quotient, remainder is in <prem>.
***************************************************************/
                                      /* +13/+3 =  4,  1 : +13 = +3 *  4 +  1 */
CELP PASCAL big_divs(quo, div, prem)  /* +13/-3 = -4,  1 : +13 = -3 * -4 +  1 */
CELP quo,div;                         /* -13/+3 = -4, -1 : -13 = +3 * -4 + -1 */
CELP *prem;                           /* -13/-3 =  4, -1 : -13 = -3 *  4 + -1 */
{                                    
    CELP res=big_div(quo,div,prem);                                  /* n1/n2 */
    TAGpart(*prem)=TAGpart(quo);
    if (TAGpart(quo)!=TAGpart(div))    /* Unequal signs so result is negative */
        TAGpart(res)=TYPE_BIGN;
    return res;
}


/***************************************************************
** NAME:        int2big
** SYNOPSIS:    CELP int2big(p)
**              CELP p;         cell with integer value
** DESCRIPTION: int2big converts a long integer to a bignumber.
** RETURNS:     The bignumber with value n.
***************************************************************/
CELP PASCAL int2big(p)
CELP p;
{
    LONG val=INTpart(p);

    p=DsGetCell(TYPE_BIGP);
    if (val<0)
    {
        TAGpart(p)=TYPE_BIGN;
        val=-val;
    }
    if (val<BIGBASE)
        BIGpart(p)=val;
    else
    {
        CELP np=DsGetCell(TYPE_BIGP);
        BIGpart(np)=val/BIGBASE;      /* quotient */
        CDRpart(p)=np;
        BIGpart(p)=val%BIGBASE;     /* remainder */
    }
    return(p);
}


/***************************************************************
** NAME:        big2int
** SYNOPSIS:    CELP big2int(p)
**              CELP p;
** DESCRIPTION: Converts if possible a bignum to an integer.
** RETURNS:     the result.
***************************************************************/
CELP PASCAL big2int(p)
CELP p;
{
    if (ISTRUE(CDRpart(p))) return p;

    {
        CELP item=DsGetCell(TYPE_INT);
        if (TAGpart(p)==TYPE_BIGN)
    	    INTpart(item)= -INTpart(p);
        else
    	    INTpart(item)= INTpart(p);
        return item;
    }
}


/***************************************************************
** NAME:        real2big
** SYNOPSIS:    CELP real2big(p)
**              CELP p;        real value
** DESCRIPTION: real2big converts a REAL number to a bignumber.
** RETURNS:     The bignumber with value n.
***************************************************************/
CELP PASCAL real2big(p)
CELP p;                   /* 1.85e30 => 1850000000000000000000000000000       */
{                              /*       1234567890123450000000000000000. 30   */
    SHORT digits,zeros;        /*      012345678901234560000000.         22   */
    CELP fp;                   /*      ^       ^       ^       12345678       */
    char fmt[10],buf[25];
    REAL val;
    int sign=1;

    val=FLTpart(p);
    if (val<0.0)
    {
        val-=val;
	sign=-1;
    }
    digits=(int)log10(val);
    zeros=0;
    if (digits>15)
    {
	zeros=digits-15;
	digits=15;
    }
    sprintf(fmt,"%%%d.0f",digits+1);
    sprintf(buf,fmt,val);
    fp = big_mul10(zeros, DsStrBig(buf));
    if (sign<0) TAGpart(fp)=TYPE_BIGN;
    return(fp);
}


/***************************************************************
** NAME:        big2real
** SYNOPSIS:    REAL big2real(p)
**              CELP p;         a bignumber.
** DESCRIPTION: big2real converts a bignumber to a REAL number.
**              At most three (most significant) parts are
**              converted.
** RETURNS:     a REAL number with value n.
***************************************************************/
REAL PASCAL big2real(p)
CELP p;                   
{                         
    REAL fres;

    if (ISNIL(p)) return 0.0;
    if (ISNIL(CDRpart(p)))                                         /* simple? */
        fres=(REAL)BIGpart(p);
    else
    {
        CELP cp;              
        int f,l,digits;

        f=digits=0;
        cp=p;
        l=(SHORT)DsLength(p);
#ifndef MATHTRAP
        if (l>308) DSERROR(ERROVRFLW,p);
#endif
        while (l>3)            /* if more than 3 cells skip least significant */
        {
            digits+=8;l--;
            cp=CDRpart(cp);
        }
        fres = (REAL)BIGpart(cp);
        while (l>1)
        {
            cp=CDRpart(cp);
            fres += ffactor[f++]*(REAL)BIGpart(cp);l--;
        }
        if (digits) fres *= pow(10.0,(REAL)digits);
    }
    if (TAG(p)==TYPE_BIGN) fres = -fres;
    return(fres);
}


/***************************************************************
** NAME:        makebig
** SYNOPSIS:    CELP makebig(arg)
**              CELP arg;       Scheme argument.
** DESCRIPTION: makebig converts a argument to a bignumber.
** RETURNS:     The bignumber with value n.
***************************************************************/
CELP PASCAL makebig(arg)
CELP arg;
{
    switch(DsGetNTag(arg))
    {
    case TYPE_INT: return int2big(arg); 
    case TYPE_FLT: return real2big(arg);
    }
    return arg;
}


/***************************************************************
** NAME:        makeflo2
** SYNOPSIS:    REAL makeflo2(arg, tag)
**              CELP arg;       Scheme argument.
**              USHORT tag;
** DESCRIPTION: makeflo2 converts a argument to a floating point
** RETURNS:     The REAL number with value n.
***************************************************************/
REAL PASCAL makeflo2(arg,tag)
CELP arg;
int tag;
{
    switch(tag)
    {
    case TYPE_INT: return (REAL)CELINT(arg); 
    case TYPE_FLT: return CELFLT(arg);       
    }
    return big2real(arg);     
}






















                
