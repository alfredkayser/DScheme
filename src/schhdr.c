/**********************************************************************
** MODULE INFORMATION*
**********************
**     FILE     NAME:       SCHHDR.C
**     SYSTEM   NAME:       SCHEME
**     ORIGINAL AUTHOR(S):  TOOHLP.EXE
**     VERSION  NUMBER:     1.0
**     CREATION DATE:       89/12/07
**                                    
** DESCRIPTION: Generated by TOOHLP. Contains functions to print (help)
**              messages.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision$
** CHANGER:     $Author$
** WORKFILE:    $Workfile$
** LOGFILE:     $Logfile$
** LOGINFO:     $Log$
**********************************************************************/
#ifdef OS2
#define INCL_VIO
#include <os2.h>
#endif
#include "schinc.h"

/*****************************************************************
** NAME:        info
** SYNOPSIS:    void info()
** DESCRIPTION: This function prints some informative text.
**              Generated by TOOHELP. (c) Kayser Systems 1990.
** RETURNS:     void
*****************************************************************/
void PASCAL info __((void)); /* prototype */

static char *address[]= {
    "Ir. Alfred Kayser",
    "Delft University of Technology",
    "Faculty of Electrical Engineering",
    "Laboratory of Computer Architecture",
    "and Digital Technique",
    "DELFT The Netherlands" } ;

#define MAXX 35
#define MAXY 6

#ifdef OS2
static char kop[] = SYSTEM" "VERSION" for "SYSTEEM;
#define KOPSIZE sizeof(kop)
#define KOPSET  ((viomi.col-KOPSIZE-4)/2)
#define OFFSET  ((viomi.col-MAXX-4)/2)
#define PUTROW(k,l,m,r,c,w) VioWrtNChar(k,1,r,c,0); \
                            VioWrtNChar(l,w+2,r,c+1,0); \
                            VioWrtNChar(m,1,r,c+w+3,0);

void PASCAL info()
{
    static BYTE abCell[2];        /* character/attribute pair */
    static VIOMODEINFO viomi;
    int i,w;

    viomi.cb=sizeof(viomi);
    VioGetMode(&viomi,0);

    abCell[0]=' ';       
    abCell[1]=15;      
    VioWrtNCell(abCell, viomi.row*viomi.col, 0, 0, 0);                
    abCell[1]=12;

    PUTROW("�","�","�", 9,OFFSET,MAXX);
    PUTROW("�","�","�", 8,KOPSET,KOPSIZE);
    PUTROW("�"," ","�", 9,KOPSET,KOPSIZE);
    PUTROW("�","�","�",10,KOPSET,KOPSIZE);
    VioWrtCharStrAtt(kop,KOPSIZE,9,KOPSET+2,&abCell[1],0);
    for (i=0;i<MAXY+1;i++)
    {
        VioWrtNChar("�",1,10+i,OFFSET,0);
        VioWrtNChar("�",1,10+i,OFFSET+MAXX+3,0);
    }
    for (i=0;i<MAXY;i++)
    {
        w=strlen(address[i]);
        VioWrtCharStr(address[i],w,11+i,(80-w)/2,0);
    }
    PUTROW("�","�","�",11+MAXY,OFFSET,MAXX);
    VioSetCurPos(23,0,0);
}
#endif

#ifdef UNIX
void PASCAL info()
{
    int i,len;
    char buf[80];

    sprintf(buf,"%s %s for %s",SYSTEM,VERSION,SYSTEEM);
    len=strlen(buf)+6;
    if (MAXX>len) len=MAXX;
    printf("\n\t\t+");
    for (i=len;i>2;i--) putchar('-');
    printf("+\n\t\t%*s\n\t\t+",(len+strlen(buf))/2,buf);
    for (i=len;i>2;i--) putchar('-');
    printf("+\n\n");
    for (i=0;i<MAXY;i++)
        printf("\t\t%*s\n",(len+strlen(address[i]))/2,address[i]);
    puts("\n\n");
}
#endif

#ifdef MSDOS
#ifdef GRAPHICS
#include <graph.h>

static struct videoconfig vc;
static struct _fontinfo fi;

void PASCAL center(int n,char *s);
short PASCAL findfont(char *env,char *name);

void PASCAL center(n,s)
int n;
char *s;
{
    _moveto((vc.numxpixels - _getgtextextent(s)) / 2,
            (vc.numypixels/2 + (n * fi.pixheight)*12/10));
    _outgtext(s);
}

void PASCAL info()
{
    short i;
    char buf[80];
    time_t end;

    sprintf(buf,"%s %s for %s",SYSTEM,VERSION,SYSTEEM);
    if ( (_registerfonts("TMSRB.FON")>0)
      && (_setvideomode(_MAXRESMODE)!=0)
      && (_setfont("t'tms rmn'h30w18bpr")>=0)
      && (_getfontinfo(&fi)>=0))
    {
        _getvideoconfig(&vc);
        _setcolor(12);
        center(-4,buf);
        _setcolor(15);
        for (i=0;i<MAXY;i++)
            center(i-2,address[i]);
        end=clock()+CLOCKS_PER_SEC*2;           /* Wait two seconds */
        while ((clock()<end) && (!kbhit()));
        _unregisterfonts();
        _setvideomode( _DEFAULTMODE );
    }
    else
    {
        puts(buf);
        for (i=0;i<MAXY;i++)
            puts(address[i]);
    }
}
#else
void PASCAL info()
{
    printf("%s %s for %s\n\n",SYSTEM,VERSION,SYSTEEM);
    puts(address[0]);
    puts(address[1]);
    puts(address[2]);
    puts(address[3]);
    puts(address[4]);
    puts(address[5]);
}
#endif
#endif
