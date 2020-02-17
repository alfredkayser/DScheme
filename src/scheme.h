/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHEME.H
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/11/9
**
** DESCRIPTION: Header file for the scheme librairy. It contains
**              all the definitions needed to call the scheme
**              functions.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   scheme.h  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHEME.H_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHEME.H_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:38:04   JAN
**              Initial revision.
**********************************************************************/

#define  S_INPORT      1
#define  S_OUTPORT     2
#define  S_STDIN       3
#define  S_STDOUT      4
#define  S_NUMCEL      5
#define  S_HASHSIZE    6
#define  S_PROMPT      7
#define  S_NUMPORT     8
#define  S_STRING      9
#define  S_STACK      10
#define  S_BUFFERS    11
#define  S_BIGBUF     12
#define  S_VERBOSE    13
#define  S_CTRLC      14 
#define  S_ERRPORT    15
#define  S_STDERR     16
#define  S_ERRFUN     17

#define  S_OKAY    0
#define  S_ARGERR  1
#define  S_ERROR   2
#define  S_NOINIT  3
#define  S_END     4
#define  S_START   5


/*----------------------------------------------------------------------------*/
/*  Defined errors reported by the SCHEME system                              */
/*----------------------------------------------------------------------------*/
#define NOERROR   0   /* End of DScheme                                       */
#define ERRMEM    1   /* Memory allocation                                    */
#define ERRINT    2   /* Internal error (should not happen)                   */
#define ERRXIT    3   /* Not an error, just an exit.                          */
#define ERRNIL    4   /* Nil pointer                                          */
#define ERRHASH   5   /* Hashtable allready initialized!                      */
#define ERRUNT    6   /* Internal: Unknown cell type                          */
#define ERRNOT    7   /* EVAL: Not implemented                                */
#define ERRARC    8   /* EVAL: Wrong argument count                           */
#define ERRART    9   /* EVAL: Wrong argument type                            */
#define ERRCAR    10  /* EVAL: Tried to take the CAR of an atom.              */
#define ERRCDR    11  /* EVAL: Tried to take the CDR of an atom               */
#define ERRVAL    12  /* EVAL: Evaluating 'define' as a atom.                 */
#define ERRPRC    13  /* EVAL: Symbol is not a procedure                      */
#define ERRSYM    14  /* EVAL: Unknown symbol                                 */
#define ERRSTK    15  /* EVAL: Stack error                                    */
#define ERRBREAK  16  /* EVAL: Abort evaluation command                       */
#define ERROUT    17  /* IO: Outbuf error (string buffer full)                */
#define ERRIOW    18  /* IO: Write error (file is readonly)                   */
#define ERRIOR    19  /* IO: Read error (file is writonly)                    */
#define ERRPRT    20  /* IO: Out of port space                                */
#define ERRCLO    21  /* IO: Port is closed!                                  */
#define ERRPORT   22  /* IO: Port table allready initialized!                 */
#define ERRPORTF  23  /* IO: Port table full                                  */
#define ERRNOF    24  /* IO: File not found                                   */
#define ERRIOU    25  /* IO: unput buffer overflow                            */
#define ERRDOT    26  /* PARSER: Dot in wrong place                           */
#define ERRSYN    27  /* PARSER: Syntax error                                 */
#define ERREOF    28  /* PARSER: Premature EOF                                */
#define ERRPARS   29  /* PARSER: Too many ')'s                                */
#define ERRBASE   30  /* PARSER: Real number don't support #o,#b,#x           */
#define ERRNUM    31  /* PARSER: Number expected after -,+                    */
#define ERRQUO    32  /* PARSER: Quote argument missing                       */
#define ERRSTR    33  /* PARSER: String too long                              */
#define ERRPINT   34  /* PARSER: Internal error                               */
#define ERRCHAR   35  /* PARSER: Illegal character encountered                */
#define ERRCHARH  36  /* PARSER: Illegal character after '#'                  */
#define ERRCHARN  37  /* PARSER: Illegal character in number                  */
#define ERRNUMF   38  /* PARSER: Illegal floating point format                */
#define ERROID    39  /* PARSER: Only integers may follow '#!'                */
#define ERRTIM    40  /* Timestamp: out of range error                        */
#define ERRTIMSYN 41  /* Timestamp: Syntax error                              */
#define ERRTIMYR  42  /* Timestamp: error in year                             */
#define ERRTIMMON 43  /* Timestamp: error in month                            */
#define ERRTIMDAY 44  /* Timestamp: error in day                              */
#define ERRTIMHR  45  /* Timestamp: error in hours                            */
#define ERRTIMMIN 46  /* Timestamp: error in minutes                          */
#define ERRTIMSEC 47  /* Timestamp: error in seconds                          */
#define ERRBIGBIG 48  /* Math: Bignumber too big (more than 2000 digits!)     */
#define ERRDIV0   49  /* Math: Divide by zero                                 */
#define ERROVRFLW 40  /* Math: Overflow                                       */
#define ERRNOBIG  51  /* Math: Bignumber math not present                     */
#define ERRFLOAT  52  /* Math: Floating point error (general)                 */
#define ERROCTAL  53  /* Parser: 3 octal digits expected                      */
#define ERRUSER1  54  /* User: User defined warning                           */
#define ERRUSER2  55  /* User: User defined error                             */
#define ERRUSER3  56  /* User: User defined serious error                     */
                      /*------------------------------------------------------*/

#define IOCLOSE 0
#define IOFLUSH 1

#define GCstart 0
#define GCstop  1
#define GCrun   2

#ifdef CMS
# define SYSTEEM "CMS on IBM-9370"
#endif
#ifdef OS2
# define SYSTEEM "OS/2"
#endif
#ifdef MSDOS
# define SYSTEEM "MS-DOS"
#endif
#ifdef UNIX
# define SYSTEEM "UNIX"
#endif
#ifdef SUN
# define SYSTEEM "SUNOS"
#endif
#ifndef SYSTEEM
#define SYSTEEM "OS"
#endif

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

#define NIL  NULL



