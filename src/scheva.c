/**********************************************************************
** MODULE INFORMATION*
**********************
**      FILE     NAME:       SCHEVA.C
**      SYSTEM   NAME:       SCHEME
**      ORIGINAL AUTHOR(S):  Alfred Kayser
**      VERSION  NUMBER:     1.5.5
**      CREATION DATE:       88/11/24
**
** DESCRIPTION: This module contains the DScheme evaluator.
***********************************************************************
** CHANGES INFORMATION **
*************************
** REVISION:    $Revision:   1.0  $
** CHANGER:     $Author:   JAN  $
** WORKFILE:    $Workfile:   scheva.c  $
** LOGFILE:     $Logfile:   C:/CPROG/SCHEME/VCS/SCHEVA.C_V  $
** LOGINFO:     $Log:   C:/CPROG/SCHEME/VCS/SCHEVA.C_V  $
**              
**                 Rev 1.0   12 Oct 1989 11:46:38   JAN
**              Initial revision.
**********************************************************************/
#include "schinc.h"
#include "schdef.h"

extern TCALL tracer;                                            /* trace hook */
CELP  key;                                             /* Current key pointer */
CELP  walker;                                                  /* List walker */

#define VARARG 0x01

STATIC CELP  PASCAL  DsReplace      __((CELP expr));
STATIC int   PASCAL  DsSearchFrame  __((CELP p));
STATIC int   PASCAL  DsSetFrame     __((CELP p, CELP name));
STATIC void  PASCAL  DsVarArg       __((void));
STATIC void  PASCAL  DsSetVar       __((CELP name));
STATIC CELP  PASCAL  DsCallExternal __((int));
STATIC CELP  PASCAL  DsApplyKernel  __((int));
STATIC void  PASCAL  DsEvalItem     __((void));
STATIC void  PASCAL  DsMakeDefine   __((void));
STATIC void  PASCAL  DsMakeProc     __((CELP formals));
STATIC void  PASCAL  DsMakeMacro    __((void));
STATIC int   PASCAL  DsValues       __((int mode));
STATIC void  PASCAL  DsLookup       __((void));
STATIC CELP  PASCAL  DsFormals      __((CELP formals, int dotted));


 /***************************************************************
 ** COMPILEBODY Makes nested defines samewhat faster, but a nested
 ** define can't access the parameters of its parent define.
 ** Example:
 ** (define (power a b)
 **    (define (square) (* a a))
 **    (* (square) b))
 ** the lambda form square can't access the parameter a of its
 ** parent. This is considered a bug and should be repaired.
 ** This is not easy. The nested defines should have there own
 ** extended env. in the at call time of the parent ext.env.
 ****************************************************************/
#ifdef COMPILEBODY
STATIC CELP  PASCAL  DsBodyCheck    __((CELP body));
#else
#define DsBodyCheck(p) p
#endif

/***************************************************************
** NAME:        DsEval
** SYNOPSIS:    CELP DsEval(arg);
**              item    The item to evaluate
** DESCRIPTION: The expression is evaluated in the current
**              environment. If it is an self-evaluating
**              expression, the item is returned 'unevalled'.
**              Otherwise is the item is passed to the real
**              evaluator. (DsEvalComplex)
** RETURNS:     The result of the expression.
***************************************************************/
CELP PASCAL DsEval(exp)
CELP exp;
{
    PUSH(item=exp);                        /* save this topexpression from GC */
    DsEvalItem();
    POP;                                         /* Done with this expression */
    return item;
}


/***************************************************************
** NAME:        DsEvalItem
** SYNOPSIS:    CELP DsEvalItem();
** DESCRIPTION: Eval_item is called by the eval routine, when
**              a complex expression is found. Eval_complex
**              checks for special forms, macro's, lambda's and
**              tail-recursivity of the expression.
** RETURNS:     The result of the expression.
***************************************************************/

STATIC
void PASCAL DsEvalItem()
{
#ifndef FIXEDGLO
    register GLOBAL *EvalGlo=PGLOBAL;
# undef GLOB
# define GLOB(x) (EvalGlo->x)
#endif

    if (ISNIL(item))
    {
        tracer(T_SELF);
        return;
    }
    if (TAGpart(item)!=TYPE_PAIR)
    {
        if (_ISVAR(item))
        {
            DsLookup();
            tracer(T_LOOKUP);
            return;
        }
        tracer(T_SELF);
        return;
    }                                              /* else complex evaluation */

    STKADD(4);                                    /* reserve some stack space */
    STKARG(4)=key;                                      /* fill a stack frame */
    STKARG(3)=GLOB(curexp);
    STKARG(2)=GLOB(curenv);                       /* Save current environment */
    STKARG(1)=GLOB(curargs);

    tracer(T_START);
    goto skipit;                                               /* quick start */

restart:
    if (ISNIL(item)) goto exit;
    if (TAGpart(item)!=TYPE_PAIR)                   /* only pairs are complex */
    {
        if (_ISVAR(item))
        {
            DsLookup();
            tracer(T_LOOKUP);
            goto exit;
        }
        tracer(T_SELF);
        goto exit;
    }
    tracer(T_TAIL);

skipit:                                       /* serious business starts here */
    GLOB(curexp)=item;                         /* remember current expression */
    key=CARpart(item);
    item=CDRpart(item);
    switch(TAG(key))
    {
    case TYPE_KEY:                                        /* kernel function? */
        if (ARGpart(key))
            if ((SHORT)DsLength(item)<ARGpart(key))
                DSERROR(ERRARC,key);
        switch (KEYpart(key))
        {                               /* Q_true points to Cel with true val */
        case IP_DEF:                           /* item= ( (sum x y) (+ x y) ) */
            DsMakeDefine();
            goto exit;

        case IP_QUOTE:
            item=CARpart(item);                /* return argument unevaluated */
            goto exit;

        case IP_LAMBDA:                         /* at least 2: ((x) (body)..) */
            key=CARpart(item);
            item=CDRpart(item);
            DsMakeProc(key);       /* turn (lambda (x y) (+ x y)) into a proc */
            goto exit;

        case IP_IF :                           /* ((test) (exp) <(else_exp)>) */
            PUSH(CDRpart(item));           /* store second part of expression */
            item=CARpart(item);                              /* get test part */
            DsEvalItem();                                    /* evaluate item */
            key=*POP;                                      /* pop second part */
            if (ISFALS(item))                          /* evaluate test expr. */
            {
                key=CDRpart(key);                       /* get third argument */
                if (ISNIL(key))                 /* else branch doesn't exist? */
                    goto exit;                            /* no, end of eval. */
            }
            item=CARpart(key);                      /* get choosen expression */
            goto restart;                    /* This enables Tail-resursivity */

        case IP_SET:                           /* 2: (A 1), rest is discarded */
            key=CARpart(item);                          /* get first argument */
            item=CADRpart(item);                       /* get second argument */
            DsEvalItem();                                    /* evaluate item */
            DsSetVar(key);
            goto exit;

        case IP_BEGIN:                 /* Item has list of expression ie body */
            item=DsBodyCheck(item);             /* Check if there are defines */
        begin_loop:
            key=item;
            if (ISNIL(key)) goto exit;
            while (1)
            {
                item=CARpart(key);                  /* get current expression */
                key=CDRpart(key);                             /* next element */
                if (ISNIL(key))  /* current expression is last one so restart */
                    goto restart;               /* function is tail resursive */
                DsEvalItem();        /* evaluate, discard result and continue */
            }

        case IP_COND:
            key=item;
            while (ISTRUE(key))
            {
                item=DsCaar(key);                           /* eval predicate */
                DsEvalItem();                 
                if (ISTRUE(item))                              /* test result */
                {
                    item=CDARpart(key);               /* evaluate this clause */
                    goto begin_loop;
                }
                key=CDRpart(key);
            }
            goto exit;

        case IP_CASE:                                    /* ( <key> <list>* ) */
            key=CDRpart(item);                             /* list of clauses */
            item=CARpart(item);                              /* eval key part */
            DsEvalItem();                                /* evaluate key part */
            while (ISTRUE(key))
            {
                walker=DsCaar(key);                       /* get first clause */
                if (walker==Q_else                             /* else clause */
                  || (ISTRUE(DsMemV(item,walker))))              /* found key */
                {
                    item=CDARpart(key);            /* get expr part of clayse */
                    goto begin_loop;                         /* evaluate this */
                }
                key=CDRpart(key);
            }
            goto exit;

        case IP_LET:
        case IP_LETA:
        case IP_LETREC:                                             /* LETREC */
            PUSH(CDRpart(item));                          /* push body of let */
            item=CARpart(item);                               /* get bindings */
            GLOB(curenv)=DsCons(DsCons1(NIL), GLOB(curenv));     /* new frame */
            if (KEYpart(key)==IP_LETREC)                     /* Recursive let */
            {
                walker=item;
                item=Q_undef;                             /* for each binding */
                while (ISTRUE(walker))
                {
                    DsDefVar(DsCaar(walker));              /* bind to *UNDEF* */
                    walker=CDRpart(walker);
                }
            }
            else
                key=item;
            while (ISTRUE(key))
            {                                          /* key ((x 10) (y 20)) */
                item=DsCadar(key);                                  /* get 10 */
                DsEvalItem();                                      /* eval it */
                DsDefVar(CAARpart(key));              /* and bind result to x */
                key=CDRpart(key);                             /* next binding */
            }
            item=*POP;                                       /* get body part */
            goto begin_loop;
                        
        case IP_MACRO:
            DsMakeMacro();                /* eval (macro (when x y) (if x y)) */
            goto exit;

        case IP_DO :                                               /* DO loop */
        case IP_DELAY:                                               /* delay */
            DSERROR(ERRNOT,key);
            goto exit;

        case IP_APPLY:
            goto restart;     /* Restart with the arguments as the expression */

        case IP_EVAL:
            switch (DsValues(0))                        /* evaluate arguments */
            {
            case 2 : TYPCHECK(CDRpart(item),TYPE_PAIR);
                     GLOB(curenv)=CDRpart(item);
            case 1 : break;
            default: DSERROR(ERRARC,key);
            }
            item=CARpart(item);                         /* get first argument */
            goto restart;

        case IP_CONS:
            DsValues(0);
            goto exit;

        case IP_LIST:
            DsValues(1);
            goto exit;

        case IP_NULL:
        case IP_NOT:
            DsValues(0);
            item=(ISNIL(item))?Q_true:NIL;
            goto exit;

        case IP_AND:
            if (ISNIL(item))
            {
                item=Q_true;
                goto exit;
            }
            key=item;
            do
            {
                item=CARpart(key);                  /* get current expression */
                key=CDRpart(key);                             /* next element */
                if (ISNIL(key))  /* current expression is last one so restart */
                    goto restart;               /* function is tail resursive */
                DsEvalItem();        /* evaluate, discard result and continue */
            }
            while (!ISFALS(item));
            goto exit;

        case IP_OR:
            key=item;
            if (ISNIL(item)) goto exit;
            do
            {
                item=CARpart(key);                  /* get current expression */
                key=CDRpart(key);                             /* next element */
                if (ISNIL(key))  /* current expression is last one so restart */
                    goto restart;               /* function is tail resursive */
                DsEvalItem();        /* evaluate, discard result and continue */
            }
            while (ISFALS(item));            /* quit on first true expression */
            goto exit;

        case IP_EXIT    :
            DSVERROR(ERRXIT);                                 /* quit DScheme */
            goto exit;

        case IP_BREAK   :
            DSVERROR(ERRBREAK);                             /* back to prompt */
            goto exit;

        case IP_VERSION :
            STRCEL(item,VERSION);
            goto exit;

        case IP_COLLECT :
            GLOB(GCflag)=3;                       /* also defrag string space */
            item=DsGetCell(TYPE_INT);          /* Get cell to store answer in */
            INTpart(item)=DsGarbageCollect(item); /* Collect, except for item */
            goto exit;

        default:                                /* Handle it as a normal form */
            item = DsApplyKernel(DsValues(0));    /* call the kernel function */
        }
        goto exit;                                      /* End of eval's task */

    case TYPE_MAC:
        walker=CAARpart(key);                              /* list of formals */
	if (ARGpart(walker))
	{
            if (KEYpart(walker) & VARARG)
	    {
                DsVarArg();                      /* rebuild list of arguments */
                if (DsLength(item)<ARGpart(walker))
                    DSERROR(ERRARC,key);
            }
            else
                if ((SHORT)DsLength(item)!=ARGpart(walker))
		    DSERROR(ERRARC,key);    
            walker=CDRpart(walker);                 /* actual list of formals */
            item=DsReplace(CDARpart(key));  /*replace formals in body by args */
        }
	else
	{
            if (item) DSERROR(ERRARC,key);                 /* No args please! */
	    item=CDARpart(key);
        }
	tracer(T_MACRO);
        goto begin_loop;                          /* eval it the official way */

    case TYPE_SPC:
        if (KEYpart(key)==IP_EOF)
            DSVERROR(ERREOF);                               /* end of input!! */
        goto exit;

    default:                             /* Evaluate first part of expression */
        {
            register int ArgLen;
                
            ArgLen=DsValues(0);                         /* evaluate arguments */
            item=key;                        
            DsEvalItem();                                /* evaluate key part */
            key=item;
            item=GLOB(curargs);
            tracer(T_APPLY);
            switch(TAG(key))               /* Apply key to arguments on stack */
            {
            case TYPE_KEY:                   /* proc part evaluated to a key? */
                if (ARGpart(key)!=ArgLen) DSERROR(ERRARC,key);
                item = DsApplyKernel(ArgLen);   /* Yep, its a kernel function */
                goto exit;                              /* End of eval's task */

            case TYPE_PRC:      /* evaluate body of procedure in extended env */
                walker=CDRpart(key);                     /* Get body and env. */
                key=CARpart(key);                             /* get PRC cell */
                if (KEYpart(key) & VARARG)
	        {
                    DsVarArg();                  /* rebuild list of arguments */
                    if (DsLength(item)<ARGpart(key))
                        DSERROR(ERRARC,key);
                }
                else
                    if (ArgLen!=ARGpart(key))
			DSERROR(ERRARC,key);    
                GLOB(curenv)=DsCons(DsCons(CDRpart(key),item),CDRpart(walker));
                item=CARpart(walker);                            /* take body */
                goto begin_loop;                             /* do begin loop */

            case TYPE_EXT:
                item = DsCallExternal(ArgLen);
                goto exit;

            default:                               /* If it ain't a user proc */
                DSERROR(ERRPRC,key);                                /* Error! */
                goto exit;
            }
        }
    }
exit:
    if (GLOB(GCflag)) DsGarbageCollect(item);                /* Clean mess up */
    GLOB(curargs)=*POP;
    GLOB(curenv)=*POP;                                 /* restore environment */
    GLOB(curexp)=*POP;
    key=*POP;
    tracer(T_END);
#ifndef FIXEDGLO
# undef GLOB
# define GLOB(x) (DsGlo->x)
#endif
}


/***************************************************************
** NAME:        DsMakeDefine
** SYNOPSIS:    void DsMakeDefine();
** DESCRIPTION: Evaluates a define statement.
** RETURNS:     void
***************************************************************/
STATIC
void PASCAL DsMakeDefine()
{
    CELP name;

    name=CARpart(item);                                    /* name= (sum x y) */
    item=CDRpart(item);                                    /* item= ((+ x y)) */
    if (ISATOM(name))                                          /* simple form */
    {
        item=CARpart(item);
        DsEvalItem();
    }
    else
    {
        DsMakeProc(CDRpart(name));                  /* turn it into procedure */
        name=CARpart(name);
    }
    DsDefVar(name);
    item=name;                                /* return defined symbol */
}


/***************************************************************
** NAME:        DsMakeProc
** SYNOPSIS:    void DsMakeProc(formals)
**              CELP formals
** DESCRIPTION: Combines the arguments of (Define ...) with the
**              current environment into a special pair called
**              TYPE_PRC. The car is the body, cdr is the env.
**              ITEM input is body, ITEM output is proc.
** RETURNS:     Pointer to new cel with the procedure.
***************************************************************/
STATIC
void PASCAL DsMakeProc(formals)
CELP formals;
{
#ifdef COMPILEBODY
    CELP elem;
    CELP curenv=GLOB(curenv);

    elem = DsBodyCheck(item); /* Body check changes curenv as a side effect!! */
    item = DsCons( DsFormals(formals, 1),
                   DsCons(elem, GLOB(curenv)));  /* Link extended environment */
    TAGpart(item) = TYPE_PRC;                                 /* Special pair */
    GLOB(curenv)=curenv;                     /* Restore to normal environment */
#else
    item = DsCons(DsFormals(formals, 1), DsCons(item, GLOB(curenv))); 
    TAGpart(item) = TYPE_PRC;                                 /* Special pair */
#endif
}


#ifdef COMPILEBODY
/***************************************************************
** NAME:        DsBodyCheck
** SYNOPSIS:    CELP DsBodyCheck(body)
**              CELP body;
** DESCRIPTION: Checks if a body starts with defines.
**              If it is then the current environment is 
**              extended to contain these defines.
**              It destroys item and key.
** RETURNS:     Stripped body
***************************************************************/
STATIC
CELP PASCAL DsBodyCheck(body)
CELP body;             /* Body such as ((define a 2) (+ a a)) */
{
    int flag=0;
    
    for (;body!=NIL;body=CDRpart(body))
    {
        CELP p;

        item=CARpart(body);             /* item is (define (sum x y) (+ x y)) */
        if (TAG(item)!=TYPE_PAIR)                   /* It isn't an expression */
            break;
        p=CARpart(item);                       /* Get key of first expression */
        if ((TAG(p)!=TYPE_KEY) || (KEYpart(p)!=IP_DEF))
            break;
        if (flag==0)                          /* Enviroment not yet extended? */
        {
            GLOB(curenv)=DsCons(DsCons1(NIL), GLOB(curenv));     /* new frame */
            flag==1;
        }
        DsEvalItem();                                  /* Evaluate the define */
    }
    return body;                                     /* Return remaining body */
}
#endif


/***************************************************************
** NAME:        DsMakeMacro
** SYNOPSIS:    void DsMakeMacro();
** DESCRIPTION: Converts a (macro ...) into a macro procedure.
**              Input and output is in ITEM.
** RETURNS:     void
***************************************************************/
STATIC
void PASCAL DsMakeMacro()
{
    CELP name;                     /* On entry is item: ((when x y) (if x y)) */

    name = CAARpart(item);                                     /* name = when */
    if (!ISSYM(name)) DSTERROR(name);
    TAGpart(name) = TYPE_MAC;                /* Make this symbol a macro name */
    CARpart(name) = DsCons( DsFormals(CDARpart(item),0),  /* make formal-list */
                            CDRpart(item));                     /* ((if x y)) */
    item=name;
}


/***************************************************************
** NAME:        DsFormals
** SYNOPSIS:    CELP DsFormals(list, dotted)
**              CELP list;
**              int dotted;
** DESCRIPTION: Converts a list of formals into a formal cell.
**              This cell contains the list, the number of
**              formals and a vararg flag.
** RETURNS:     void
***************************************************************/
STATIC CELP PASCAL DsFormals(formals, dot)
CELP formals;
int dot;
{
    CELP p,q;
    int dotted=FALSE, len=0;
    
    q=NIL;
    p=formals;
    while (p!=NIL)
    {
        len++;
        if (TAGpart(p)!=TYPE_PAIR)
        {
            dotted=VARARG;
            if (ISTRUE(q))
                CDRpart(q)=dot?p:DsCons1(p);     /* Add last item to the list */
            break;
        }
        if (CDRpart(p)==NIL)                              /* End of this list */
        {
            p=CARpart(p);
            if (!dot) p=DsCons1(p);
            if (ISTRUE(q))
                CDRpart(q)=p;                    /* Add last item to the list */
            else
                formals=p;                 /* Get the first and only argument */
            break;
        }
        if (ISTRUE(q))
            q=CDRpart(q)=DsCons1(CARpart(p));
        else
            formals=q=DsCons1(CARpart(p));
        p=CDRpart(p);
    }
    p=DsGetCell(TYPE_FUN);
    CDRpart(p)=formals;    
    KEYpart(p)=dotted;
    ARGpart(p)=len;
    return p;
}


/***************************************************************
** NAME:        DsReplace
** SYNOPSIS:    CELP DsReplace(expr)
**              CELP expr;
** DESCRIPTION: Copies a expression, while replacing all symbols
**              indicated by 'DsReplace_symbols', by the new
**              values stored in 'DsReplace_newvals'.
** RETURNS:     Pointer to new expression
***************************************************************/
STATIC
CELP PASCAL DsReplace(expr)
CELP expr;
{
    if (ISNIL(expr)) return NIL;
    switch (TAGpart(expr))
    {
    case TYPE_PAIR:
        return DsCons( DsReplace(CARpart(expr)),
                       DsReplace(CDRpart(expr)));
    case TYPE_SYMD:
    case TYPE_SYM:
        {
            static CELP p,q;            /* they don't have to be on the stack */
            p=walker;
            q=item;
            while (ISTRUE(p))
            {
                if (CARpart(p)==expr)                       /* found a symbol */
                    return(CARpart(q));            /* replace it by new value */
                p=CDRpart(p);
                q=CDRpart(q);
            }
        }
    }
    return(expr);                                 /* don't have to replace it */
}


/***************************************************************
** NAME:        DsValues
** SYNOPSIS:    int DsValues(void);
** DESCRIPTION: This function evaluates each element of the list
**              of arguments. Input and output is ITEM.
** RETURNS:     number of arguments.
***************************************************************/
STATIC
int PASCAL DsValues(mode)
int mode;
{
    if (ISNIL(item))
    {
        GLOB(curargs)=NIL;
        return 0;
    }
    else
    {
        int len=1;
        CELP cp,args=item;                /* These must be on the stack! */
 
        if (TAGpart(args)!=TYPE_PAIR) DSTERROR(args);
        item=CARpart(args);                      /* Get argument from list */
        DsEvalItem();                            /* Evaluate item */ 
        args=CDRpart(args);                      /* Goto next argument */
        if (mode)                                /* Build a normal list */
        {
            GLOB(curargs)=cp=DsCons1(item);      /* Store first value */
            while (ISTRUE(args)) 
            {
                if (TAGpart(args)!=TYPE_PAIR) DSTERROR(args);
                item=CARpart(args);               /* Get argument from list */
                DsEvalItem();                     /* Evaluate item */ 
                cp=CDRpart(cp)=DsCons1(item);
                args=CDRpart(args);               /* Goto next argument */
                len++;
            }
        }       
        else
        {
            if (ISNIL(args))       /* End of list, so there is one arg */
            {
                GLOB(curargs)=cp=item;
                return 1;
            }
            GLOB(curargs)=cp=DsCons1(item);   /* Store first value */
            len=2;
            while (1)
            {
                /* dotted args lists are not implemented... */
                if (TAGpart(args)!=TYPE_PAIR) DSTERROR(args);
                item=CARpart(args);           /* Get argument from list */
                DsEvalItem();                 /* Evaluate item */ 
                args=CDRpart(args);           /* Goto next argument */
                if (ISNIL(args)) break;       /* No more args, break */
                cp=CDRpart(cp)=DsCons1(item); /* Store middle values */
                len++;
            }
            cp=CDRpart(cp)=item;              /* Store last value */
        }
        item=GLOB(curargs);
        return len;
    }
}


/**************************************************************
** NAME:        DsSearchFrame
** SYNOPSIS:    int DsSearchFrame(p)
**              CELP p;
** DESCRIPTION: Search in frame pointed by p, for the cel in
**              which <item> is bound. 
** RETURNS:     True, value is in item 
**              False, none found.
**************************************************************/
STATIC
int PASCAL DsSearchFrame(p)
CELP p;
{
    static CELP fp;                        /* Doesn't have to be on the stack */

    p=CARpart(p);                                       /* Point to the frame */
    for (fp=CARpart(p); ISPAIR(fp); fp=CDRpart(fp))
    {
        p=CDRpart(p);
        if (CARpart(fp)==item)
        {
            item = CARpart(p);               /* Found it */
            return TRUE;
        }
    }
    if (fp==item)
    {
        item=CDRpart(p);
        return TRUE;
    }
    return FALSE; 
}


/**************************************************************
** NAME:        DsSetFrame
** SYNOPSIS:    int DsSetFrame(p, name)
**              CELP p;
**              CELP name;
** DESCRIPTION: Search in frame pointed by <p> for <name>. 
**              If it is found it is bound to <item>.
** RETURNS:     TRUE, name found and bound to item.
**              FALSE, name not found,
**************************************************************/
STATIC
int PASCAL DsSetFrame(p,name)
CELP p;
CELP name;
{
    static CELP fp;                        /* Doesn't have to be on the stack */

    p=CARpart(p);                /* Point to the frame itself */
    for (fp=CARpart(p); ISPAIR(fp); fp=CDRpart(fp))  /* Walk list of formals */
    {
        p=CDRpart(p);
        if (CARpart(fp)==name)
        {
            CARpart(p)=item;
            return TRUE;
        }
    }
    if (fp==name)
    {
        CDRpart(p)=item;
        return TRUE;
    }
    return FALSE;
}


/***************************************************************
** NAME:        DsLookup
** SYNOPSIS:    CELP DsLookup();
** DESCRIPTION: Finds the symbol ITEM in the current environment
**              Calls DSERROR if symbol isn't defined.
** RETURNS:     Pointer to value of symbol.
***************************************************************/
STATIC
void PASCAL DsLookup()
{
    static CELP q;                         /* Doesn't have to be on the stack */

    /* Search environment first */
    for (q=GLOB(curenv); ISTRUE(q); q=CDRpart(q))
        if (DsSearchFrame(q))
           return;
    switch(TAGpart(item))                                    /* Special cases */
    {
    case TYPE_KEY: return;
    case TYPE_SYMD: item=CARpart(item);return;
    }
    DSERROR(ERRSYM,item);                                      /* Not found ! */
}


/***************************************************************
** NAME:        DsDefVar
** SYNOPSIS:    void DsDefVar(name)
**              CELP name;      symbol to be defined.
** DESCRIPTION: Defines the symbol in the current environment.
**              If the symbol already exists, the new value is
**              bound to it. The value is passed via ITEM.
** RETURNS:     void
***************************************************************/
void PASCAL DsDefVar(name)
CELP name;
{
    if (!ISSYM(name)) DSTERROR(name);
    switch(TAG(item))                                  /* Check special cases */
    {
    case TYPE_MAC:
        TAGpart(name)=TYPE_MAC;
        CARpart(name)=CARpart(item);                 /* copy macro definition */
        return;

    case TYPE_KEY:
        TAGpart(name)=TYPE_KEY;
        KEYpart(name)=KEYpart(item);                   /* copy keyword number */
        ARGpart(name)=ARGpart(item);
        return;
    }
    if (GLOB(curenv)==GLOB(sysenv))                     /* We're at top level */
    {
        TAGpart(name)=TYPE_SYMD;                          /* a defined symbol */
        CARpart(name)=item;                                  /* bind to value */
    }
    else
    {
        if (TAGpart(name)!=TYPE_SYMD)
            TAGpart(name)=TYPE_SYM;           /* it's now defined as a symbol */
        if (!DsSetFrame(GLOB(curenv), name))         /* Not in current frame? */
        {
            static CELP p;
        
            p=CARpart(GLOB(curenv));        /* extend environment */
            if (ISNIL(CARpart(p)))
            {
                    CARpart(p)=name;
                    CDRpart(p)=item;
            }
            else
            {
                    CARpart(p)=DsCons(name,CARpart(p));
                    CDRpart(p)=DsCons(item,CDRpart(p));
            }
        }
    }
}


/***************************************************************
** NAME:        DsSetVar
** SYNOPSIS:    void DsSetVar(name)
**              CELP name;      symbol to be set.
** DESCRIPTION: Rebounds the symbol with the new value stored
**              in ITEM. Returns the bound value in ITEM.
**              If the symbol doesn't exists, DsError is called.
** RETURNS:     Returns the bound value.
***************************************************************/
STATIC
void PASCAL DsSetVar(name)
CELP name;
{
    if (!ISSYM(name)) DSTERROR(name);
    switch(TAG(item))                                  /* Check special cases */
    {
    case TYPE_MAC:
        TAGpart(name)=TYPE_MAC;
        CARpart(name)=CARpart(item);                 /* copy macro definition */
        return;

    case TYPE_KEY:
        TAGpart(name)=TYPE_KEY;
        KEYpart(name)=KEYpart(item);                   /* copy keyword number */
        ARGpart(name)=ARGpart(item);                   /* copy number of args */
        return;
    }
    /* default */
    {
        static CELP env;                   /* Doesn't have to be on the stack */
        for (env=GLOB(curenv); ISTRUE(env); env=CDRpart(env))
        {
            if (DsSetFrame(env, name))            /* Search and bind if found */
            {
                if (TAGpart(name)!=TYPE_SYMD)
                    TAGpart(name)=TYPE_SYM;   /* it's now defined as a symbol */
                return;                                    /* Found and bound */
            }
        }                                         /* not found in environment */
        if (TAGpart(name)!=TYPE_SYMD)                 /* defined in toplevel? */
            DSERROR(ERRSYM,name);    /* Oops, we can only set defined symbols */
        CARpart(name)=item;                       /* redefine toplevel symbol */
    }
}


/**************************************************************
** NAME:        DsVarArg
** SYNOPSIS:    void DsVarArg()
** DESCRIPTION: Rebuilds a list of arguments of a lambda call
**              with dotted list of formals to a simple list.
** RETURNS:     void
**************************************************************/
STATIC
void PASCAL DsVarArg()
{
    if (!ISPAIR(item))
        item=DsCons1(item);
    else
    {
        static CELP p,q;
        q=item;
        for (p=CDRpart(q); ISPAIR(p); p=CDRpart(p))
            q=p;
        CDRpart(q)=DsCons1(p);
    }
}


/***************************************************************
** NAME:        DsCallExternal
** SYNOPSIS:    CELP DsCallExternal(ArgLen);
**              int ArgLen;
** DESCRIPTION: Calls an user supplied C-procedure.
**              This procedure can be linked to DScheme via the
**              DSextdef function.
**              KEY points to key cell.
** RETURNS:     The result of the external function
** SEE ALSO:    DSextdef
***************************************************************/
typedef CELP (* FUNC0) __((void));      /* external defined procedures */
typedef CELP (* FUNC2) __((CELP,CELP)); /* external defined procedures */
typedef CELP (* FUNC3) __((CELP,CELP,CELP)); /* external defined procs */
typedef CELP (* FUNCN) __((int,CELP));  /* external defined procedures */
typedef CELP (* FUNC1) __((CELP));      /* external defined procedures */

STATIC
CELP PASCAL DsCallExternal(ArgLen)
int ArgLen;
{
    register EXTDEF *extdef;

    extdef=CELEXT(key);
    if (extdef->args==-1)                /* don't want to check on arg count? */
        return ((FUNCN)extdef->extrn)(ArgLen,item);
    if (extdef->args==ArgLen)                              /* check arg count */
    {
        static CELP arg1, arg2;                  /* static's are not on stack */
        switch(extdef->args)
        {
        case 0:
            return ((FUNC0)extdef->extrn)();
        case 1:
            if (extdef->at1) TYPCHECK(item,extdef->at1);
            return ((FUNC1)extdef->extrn)(item);
        case 2:
            arg1=CARpart(item);
            item=CDRpart(item);
            if (extdef->at1) TYPCHECK(arg1,extdef->at1);
            if (extdef->at2) TYPCHECK(item,extdef->at2);
            return ((FUNC2)extdef->extrn)(arg1, item);
        case 3:
            arg1=CARpart(item);item=CDRpart(item);
            arg2=CARpart(item);
            item=CDRpart(item);
            if (extdef->at1) TYPCHECK(arg1,extdef->at1);
            if (extdef->at2) TYPCHECK(arg2,extdef->at2);
            if (extdef->at3) TYPCHECK(item,extdef->at3);
            return ((FUNC3)extdef->extrn)(arg1,arg2,item);
        }
    }
    DSERROR(ERRARC,key);
}


/***************************************************************
** NAME:        DsApplyKernel
** SYNOPSIS:    CELP DsApplyKernel();
** DESCRIPTION: This function applies the procedure on the
**              given arguments. Input via ITEM and KEY.
** RETURNS:     The result value
***************************************************************/
STATIC
CELP PASCAL DsApplyKernel(ArgLen)
int ArgLen;
{               
    switch(ARGpart(key))
    {
    case 1:
        return Ds_math1(KEYpart(key),GLOB(curargs));

    case 2:
        {
            CELP q=CDRpart(GLOB(curargs));
            item=CARpart(GLOB(curargs));
            switch(KEYpart(key))
            {
            case IP_CHARLT  : return DsCmpChar(q,item,2);
            case IP_CHARLE  : return DsCmpChar(q,item,1);
            case IP_CHAREQ  : return DsCmpChar(item,q,0);
            case IP_CHARGE  : return DsCmpChar(item,q,1);
            case IP_CHARGT  : return DsCmpChar(item,q,2);
            case IP_CHARLTCI  : return DsCmpChar(q,item,6);
            case IP_CHARLECI  : return DsCmpChar(q,item,5);
            case IP_CHAREQCI  : return DsCmpChar(item,q,4);
            case IP_CHARGECI  : return DsCmpChar(item,q,5);
            case IP_CHARGTCI  : return DsCmpChar(item,q,6);
            case IP_SETCAR  : CARpart(item)=q; return item;
            case IP_SETCDR  : CDRpart(item)=q; return item;                      
            case IP_ASSQ    : return DsAssQ(item,q);
            case IP_ASSV    : return DsAssV(item,q);
            case IP_ASSOC   : return DsAssoc(item,q);
            case IP_EQ      : return TEST(item==q);
            case IP_EQV     : return DsEqv(item,q);
            case IP_EQUAL   : return DsEqual(item,q);
            case IP_MEMQ    : return DsMemQ(item,q);
            case IP_MEMV    : return DsMemV(item,q);
            case IP_MEMBER  : return DsMember(item,q);
            case IP_TIMEEQ  : return DsCmpTime(item,q,0);
            case IP_TIMELE  : return DsCmpTime(q,item,1);
            case IP_TIMELT  : return DsCmpTime(q,item,2);
            case IP_TIMEGE  : return DsCmpTime(item,q,-1);
            case IP_TIMEGT  : return DsCmpTime(item,q,-2);
            }
        }
    }
    DSERROR(ERRNOT,key);
}

