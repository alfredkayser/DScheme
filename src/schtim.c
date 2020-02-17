/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHTIM.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/05/10
**
** DESCRIPTION: This module defines some routines to handle Julian
**              Date numbers. These numbers store any date from
**              January 1, 1900 to December 31, 2078 in only two
**              bytes.
**              The available routines support conversion to/from
**              a day, month and year format to the Julian Date
**              format, checking for valid dates, adding a number
**              of days, months and years to a Date, conversion
**              to/from a Date and a routine for determining the
**              day of the week for a given date.
**              The advantages of using Julian Date numbers are
**              the savings of much storage space (2 bytes vs.
**              6 ASCII characters) and that the number of days
**              between any two dates is a simple subtraction.
**              The algorithms used here are adapted from an
**              article by Gordon King in the June 1983 issue of
**              Dr. Dobb's Journal. Mr. King in turn credits
**              Algorithm 199 in "The Collected Algorithms of the
**              ACM" (1963) by R. G. Tantzen.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schtim.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHTIM.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHTIM.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:45:52   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"

static CONST char * PASCAL  getdigits   __((CONST char *p, int n, int *result));

/***************************************************************
** NAME:        DsTimeStr
** SYNOPSIS:    char *DsTimeStr(elem)
**              CELP elem;      A SCHEME timestamp.
** DESCRIPTION: DsTimestr converts a timestamp into a printable
**              and (DScheme) readable string.
**              Format: #@1988/11/22-12:34:56.78
** RETURNS:     A pointer to a buffer (bigbuf) which contains the
**              string version of the timestamp.
**              Be sure to copy this string before the next call
**              to DsTimeStr.
***************************************************************/
char * PASCAL DsTimeStr(elem)
CELP elem;
{
    int y,m,d;
    if (CELTIM(elem).date!=0)
    {
        DsSetDays(CELTIM(elem).date,&d,&m,&y);
        sprintf(BIGBUF,"#@%4d/%02d/%02d-%d:%02d:%02g",
                             y,m,d,TIME_H(elem->dat.ts.time),
                             TIME_M(elem->dat.ts.time),
                             elem->dat.ts.fsec);
    }
    else
    {
        sprintf(BIGBUF,"#@-%d:%02d:%02g",
                             TIME_H(elem->dat.ts.time),
                             TIME_M(elem->dat.ts.time),
                             elem->dat.ts.fsec);
    }
    return(BIGBUF);
}


/***************************************************************
** NAME:        DsStrTime
** SYNOPSIS:    CELP DsStrTime(str);
**              char *str;      the string with the timestamp
** DESCRIPTION: StrTime converts a timestamp and checks it.
**              Format: #@1988/11/22-12:34:56.789
**                  or: #@1988/11/22
**                  or: #@-12:34:56.789
**                  or: #@-12:34
**                  or: #@-::0.003      (3 millesecs)
** RETURNS:     A new cell containing the timestamp.
***************************************************************/
CELP PASCAL DsStrTime(str)
CONST char *str;
{
    int  year,mon,day,hour,minu,sec;
    int  date;
    float secf;
    CONST char *p;
    CELP tc;

    secf=(float)0.0;
    date=year=mon=day=hour=minu=sec=0;
    p=str+2;
    if (*p!='-')
    {                                   /* Get date */
        p=getdigits(p,4,&year);           /* skip "#@" */
        if (year<100)
        {
            if (year>80)
                year+=1900;                        /* used two digit notation */
            else
                year+=2000;                  /* two digit in 2010: #@10/12/31 */
        }
        if (year<1980 || year>2078)
            DsStrError(ERRTIMYR,str);
        if (*p!='/') DsStrError(ERRTIMMON,str);
        p=getdigits(++p,2,&mon);
        if (mon>12) DsStrError(ERRTIMMON,str);
        if (*p!='/') DsStrError(ERRTIMDAY,str);
        p=getdigits(++p,2,&day);
        if (day>31) DsStrError(ERRTIMDAY,str);

        date = DsGetDays(day,mon,year);
    }
    if (*p)                                                   /* follows more */
    {
        if (*p++!='-')                                /* This should come now */
            DsStrError(ERRTIMSYN,str);

        if (*p!=':')                              /* The hours are coming now */
        { 
            p=getdigits(p,2,&hour)+1;
            if (hour>23) DsStrError(ERRTIMHR,str);
        }
        if (*p!=':')                                        /* Now minutes... */
        {
            p=getdigits(p,2,&minu);
            if (minu>59) DsStrError(ERRTIMMIN,str);
        }
        if (*p)                         /* still some timestamp left, seconds */
        {
            secf=(float)strtod(p+1,&p);
            if (*p!=0)/* not at end of string or RANGE error */
                DsStrError(ERRTIMSEC,str);
        }
    }
    if (*p!='\0')                                             /* garbage left */
        DsStrError(ERRTIMSYN,str);
    INITCEL(tc,TYPE_TMS);
    CELTIM(tc).fsec = secf;
    CELTIM(tc).time = COM_TIME(hour,minu);
    CELTIM(tc).date = date;
    return(tc);
}


/***************************************************************
** NAME:        getdigits
** SYNOPSIS:    char *getdigits(p,n,result);
**              CONST char *p;        Pointer to a string of digits
**              int  n;         Number of digits to read.
**              int  *result;   Pointer to result value.
** DESCRIPTION: Getdigits converts a string of max. n digits to
**              a integer value.
** RETURNS:     A pointer to the first non processed character.
**              This can the first non digit or after n chars.
***************************************************************/
static
CONST char * PASCAL getdigits(p,n,result)
CONST char *p;
int n,*result;
{
    register int r=0;
    while ((isdigit(*p))&&(n--))
         r = r*10 + *p++ -'0';
    *result=r;
    return(p);
}


/***************************************************************
** NAME:        DsTimeReal
** SYNOPSIS:    CELP DsTimeReal(p)
**              CELP p;         pointer to timestamp cell
** DESCRIPTION: Convert a timestamp to a unique sortable
**              floating point number.
** RETURNS:     Cell with ((date*65535)+time)*60+seconds
** SEE ALSO:    real2time
***************************************************************/
CELP PASCAL DsTimeReal(p)
CELP p;
{
    CELP cp;
    FLTCEL(cp, 60.0*(REAL)((1440L*CELTIM(p).date)+(LONG)CELTIM(p).time)
               + (REAL)CELTIM(p).fsec);
    return(cp);
}


/***************************************************************
** NAME:        DsRealTime
** SYNOPSIS:    CELP DsRealTime(f)
**              CELP f;         Floating point cell
** DESCRIPTION: Converts a floating point number to a timestamp.
**              Inversive version of DsRealTime means:
**              timestamp=DsRealTime(DsTimeReal(timestamp)).
**              Except that a new cell is allocated, but is
**              contents is the same.
** RETURNS:     pointer to timestamp.
** SEE ALSO:    DsTimeReal
***************************************************************/
CELP PASCAL DsRealTime(p)
CELP p;
{
    REAL tmp,f;
    LONG mins;

    f=CELFLT(p);            /* get value to convert */
    p=DsGetCell(TYPE_TMS);     /* new timestamp */
    tmp=fmod(f,60.0);
    mins=(LONG)floor(f/60.0);
    CELTIM(p).fsec=(float)tmp;
    CELTIM(p).time=(WORD)(mins%1440L);
    CELTIM(p).date=(WORD)(mins/1440L);
    if (CELTIM(p).time>COM_TIME(23,59) || CELTIM(p).time<0)
        DSERROR(ERRTIM,p);
    return(p);
}


/*****************************************************************
** NAME:        DsGetDays
** SYNOPSIS:    WORD DsGetDays(day,month,year)
**              int day,month,year;
** DESCRIPTION: Calculates the Julian day number for the given
**              date. Note that no error checking is taking place
**              in this routine -- if the date given doesn't make
**              sense then neither will the result, use ValidDate
**              to check for valid dates first if necessary.
** RETURNS:     Number of days
*****************************************************************/
WORD PASCAL DsGetDays(day,month,year)    /* Julian Day Number - Jul(1900/1/1) */
int day,month,year;
{
    WORD julian;

    if (year==0) return(0);                                   /* No year zero */
    if ((year==1900) && (month<3))
    {
        if (month==1)
            julian = day-1;
        else
            julian = day+30;
    }
    else
    {
        if (month>2)
            month-=3;
        else
        {
            month+=9;
            year--;
        }
        year-=1900;
        julian = (int)((1461L*(LONG)year) >> 2)
               + ((153*month+2)/5)
               + day
               + 58;
    }
    return(julian);
}


/*****************************************************************
** NAME:        DsSetDays
** SYNOPSIS:    void DsSetDays(julian,day,month,year)
**              int julian,*day,*month,*year;
** DESCRIPTION: The reciprocal routine for DsGetDays, this procedure
**              takes a Julian day number and returns the day,
**              month and year.
** RETURNS:     void
*****************************************************************/
void PASCAL DsSetDays(julian,day,month,year)
WORD julian;
int *day, *month, *year;
{
    LONG temp;
    int tmp;

    if (julian<=58)
    {
        *year=1900;
        if (julian <= 30)
        {
            *month = 1;
            *day= julian+1;
        }
        else
        {
            *month = 2;
            *day = julian-30;
        }
    }
    else
    {
        temp = ((LONG)julian<<2) - 233L;
        *year = 1900+(int)(temp/1461L);
        tmp = ((int)(temp%1461L) >>2) * 5 + 2;
        *day = (tmp%153)/5 + 1;
        tmp /= 153;
        if (tmp<10)
            *month=tmp+3;
        else
        {
            *month=tmp-9;
            year++;
        }
    }
}


/*****************************************************************
** NAME:        DsDayOfWeek
** SYNOPSIS:    int DsDayOfWeek(julian);
**              int julian;
** DESCRIPTION: Return the day of the week for the date.
** RETURNS:     day of week (0-6)
*****************************************************************/
int PASCAL DsDayOfWeek(julian)
int julian;
{
    return((++julian) % 7);
}


CELP PASCAL DsMakeTime(t)
time_t t;
{
    struct tm *lt;
    CELP p;

    lt=localtime(&t);
    INITCEL(p,TYPE_TMS);
    CELTIM(p).date = DsGetDays(lt->tm_mday,lt->tm_mon+1,lt->tm_year+1900);
    CELTIM(p).time = COM_TIME(lt->tm_hour,lt->tm_min);
    CELTIM(p).fsec = (float)lt->tm_sec;
    return p;
}

