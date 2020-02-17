/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHDEF.H
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       89/21/31
**                                                                  
** DESCRIPTION: This file contains the definitions of the kernel
**              functions
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schdef.h  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHDEF.H_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHDEF.H_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:39:38   JAN
**              Initial revision.
**********************************************************************/

/**--------------------------------------------------------------**/
/**  The IP number indicates what action should be taken when    **/
/**  eval wants to evaluate this element.                        **/
/**--------------------------------------------------------------**/

                   /*30*/
#define IP_ASSQ      31
#define IP_ASSV      32	
#define IP_ASSOC     33
#define IP_SETCAR    34
#define IP_SETCDR    35

#define IP_CHAREQ    36
#define IP_CHARLT    37
#define IP_CHARLE    38
#define IP_CHARGT    39
#define IP_CHARGE    40
#define IP_CHAREQCI  41
#define IP_CHARLTCI  42
#define IP_CHARLECI  43
#define IP_CHARGTCI  44
#define IP_CHARGECI  45

#define IP_MEMQ      46
#define IP_MEMV      47
#define IP_MEMBER    48
#define IP_EQ        49
#define IP_EQV       50
#define IP_EQUAL     51

#define IP_TIMEEQ    52
#define IP_TIMELT    53
#define IP_TIMELE    54
#define IP_TIMEGT    55
#define IP_TIMEGE    56

#define IP_ISTIME    57
#define IP_ISSYM     58
#define IP_ISATOM    59
#define IP_ISBOOL    60
#define IP_ISLIST    61
#define IP_ISPROC    62
#define IP_ISCHAR    63
#define IP_ISPORT    64
#define IP_ISBIGNUM  65
#define IP_ISRAT     66
#define IP_COMPLEX   67
#define IP_ISNUMBER  68
#define IP_ISPAIR    69
#define IP_ISSTR     70
#define IP_ISEOF     71
#define IP_EXACT     72
#define IP_ISOPORT   73
#define IP_INEXACT   74
#define IP_ISIPORT   75
#define IP_ISINT     76
#define IP_ISREAL    77
#define IP_ISVECTOR  78
                    
#define IP_ISCHALPHA 79
#define IP_ISCHNUMER 80
#define IP_ISCHWHITE 81
#define IP_ISCHUPERR 82
#define IP_ISCHLOWER 83

#define IP_MAXCONST  84

