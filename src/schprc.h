/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHPRC.H
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/03/21
**
** DESCRIPTION: This module contains definitions of the essential
**              and some optional scheme functions.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   schprc.h  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHPRC.H_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHPRC.H_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:43:28   JAN
**              Initial revision.
**********************************************************************/

#include "schdef.h"

struct {
       char *name;
       int nargs;
       } funs[]= {

/** IP_EOF       **/ { "*EOF*",                 0 },  /*  0 */
/** IP_DEFINE    **/ { "DEFINE",                2 },  /*  1 */
/** IP_QUOTE     **/ { "QUOTE",                 1 },  /*  2 */
/** IP_LAMBDA    **/ { "LAMBDA",                2 },  /*  3 */
/** IP_IF        **/ { "IF",                    2 },  /*  4 */
/** IP_SET       **/ { "SET!",                  2 },  /*  5 */
/** IP_BEGIN     **/ { "BEGIN",                 0 },  /*  6 */
/** IP_COND      **/ { "COND",                  0 },  /*  7 */
/** IP_CASE      **/ { "CASE",                  2 },  /*  8 */
/** IP_LET       **/ { "LET",                   2 },  /*  9 */
/** IP_LETA      **/ { "LET*",                  2 },  /* 10 */
/** IP_LETREC    **/ { "LETREC",                2 },  /* 11 */
/** IP_MACRO     **/ { "MACRO",                 2 },  /* 12 */
/** IP_DO        **/ { "DO",                    2 },  /* 13 */
/** IP_DELAY     **/ { "DELAY",                 1 },  /* 14 */
/** IP_APPLY     **/ { "APPLY",                 2 },  /* 15 */
/** IP_EVAL      **/ { "EVAL",                  1 },  /* 16 */
/** IP_CONS      **/ { "CONS",                  2 },  /* 17 */
/** IP_LIST      **/ { "LIST",                  0 },  /* 18 */
/** IP_NULL      **/ { "NULL?",                 1 },  /* 19 */
/** IP_NOT       **/ { "NOT",                   1 },  /* 20 */
/** IP_AND       **/ { "AND",                   0 },  /* 21 */
/** IP_OR        **/ { "OR",                    0 },  /* 22 */
/** IP_EXIT      **/ { "EXIT",                  0 },  /* 23 */
/** IP_BREAK     **/ { "BREAK",                 0 },  /* 24 */
/** IP_COLLECT   **/ { "VERSION",               0 },  /* 25 */
/** IP_COLLECT   **/ { "COLLECT",               0 },  /* 26 */
/** IP_TRUE      **/ { "#T",                    0 },  /* 27 */
/** IP_INVIS     **/ { "",                      0 },  /* 28 */
/** IP_UNDEF     **/ { "*UNDEFINED*",           0 },  /* 29 */
/** IP_ELS       **/ { "ELSE",                  0 },  /* 30 */

/** IP_ASSQ      **/ { "ASSQ",                  2 },
/** IP_ASSV      **/ { "ASSV",                  2 },
/** IP_ASSOC     **/ { "ASSOC",                 2 },

/** IP_SETCAR    **/ { "SET-CAR!",              2 },
/** IP_SETCDR    **/ { "SET-CDR!",              2 },
/** IP_CHAREQ    **/ { "CHAR=?",                2 },
/** IP_CHARLT    **/ { "CHAR<?",                2 },
/** IP_CHARLE    **/ { "CHAR<=?",               2 },
/** IP_CHARGT    **/ { "CHAR>?",                2 },
/** IP_CHARGE    **/ { "CHAR>=?",               2 },
/** IP_CHAREQCI  **/ { "CHAR-CI=?",             2 },
/** IP_CHARLTCI  **/ { "CHAR-CI<?",             2 },
/** IP_CHARLECI  **/ { "CHAR-CI<=?",            2 },
/** IP_CHARGTCI  **/ { "CHAR-CI>?",             2 },
/** IP_CHARGECI  **/ { "CHAR-CI>=?",            2 },

/** IP_MEMQ      **/ { "MEMQ",                  2 },
/** IP_MEMV      **/ { "MEMV",                  2 },
/** IP_MEMBER    **/ { "MEMBER",                2 },
/** IP_EQ        **/ { "EQ?",                   2 },
/** IP_EQV       **/ { "EQV?",                  2 },
/** IP_EQUAL     **/ { "EQUAL?",                2 },

/** IP_TIMEEQ    **/ { "TIME=?",                2 },
/** IP_TIMELT    **/ { "TIME<?",                2 },
/** IP_TIMELE    **/ { "TIME<=?",               2 },
/** IP_TIMEGT    **/ { "TIME>?",                2 },
/** IP_TIMEGE    **/ { "TIME>=?",               2 },

/** IP_ISTIME    **/ { "TIME?",                 1 },
/** IP_ISSYM     **/ { "SYMBOL?",               1 },  
/** IP_ISATOM    **/ { "ATOM?",                 1 },
/** IP_ISBOOL    **/ { "BOOLEAN?",              1 },
/** IP_ISLIST    **/ { "LIST?",                 1 },
/** IP_ISPROC    **/ { "PROCEDURE?",            1 },
/** IP_ISCHAR    **/ { "CHAR?",                 1 },
/** IP_ISPORT    **/ { "PORT?",                 1 },
/** IP_ISBIGNUM  **/ { "BIGNUM?",               1 },
/** IP_ISRAT     **/ { "RATIONAL?",             1 },
/** IP_COMPLEX   **/ { "COMPLEX?",              1 },
/** IP_ISNUMBER  **/ { "NUMBER?",               1 },
/** IP_ISPAIR    **/ { "PAIR?",                 1 },
/** IP_ISSTR     **/ { "STRING?",               1 },
/** IP_ISEOF     **/ { "EOF-OBJECT?",           1 },
/** IP_EXACT     **/ { "EXACT?",                1 },
/** IP_ISOPORT   **/ { "OUTPUT-PORT?",          1 },
/** IP_INEXACT   **/ { "INEXACT?",              1 },
/** IP_ISIPORT   **/ { "INPUT-PORT?",           1 },
/** IP_ISINT     **/ { "INTEGER?",              1 },
/** IP_ISREAL    **/ { "REAL?",                 1 },
/** IP_ISVECTOR  **/ { "VECTOR?",               1 },

/** IP_ISCHALPHA **/ { "CHAR-ALPHABETIC?", 1},
/** IP_ISCHNUMER **/ { "CHAR-NUMERIC?", 1 },
/** IP_ISCHWHITE **/ { "CHAR-WHITESPACE?", 1 },
/** IP_ISCHUPERR **/ { "CHAR-UPPER-CASE?", 1 },
/** IP_ISCHLOWER **/ { "CHAR-LOWER-CASE?", 1 },

};

