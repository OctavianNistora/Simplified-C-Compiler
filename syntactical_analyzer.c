#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "lexical_analyzer.h"
#include "syntactical_analyzer.h"
#include "semantic_analyzer_fragment.h"

Token *crtTk;
Token *consumedTk;

Token *getCrtTk()
{
    return crtTk;
}

int consume(int code)
{
    if (getTokenCode(crtTk) == code)
    {
        consumedTk = crtTk;
        crtTk = getTokenNext(crtTk);
        return 1;
    }
    return 0;
}

int declStruct();
int declFunc();
int declVar();

int unit()
{
    while (declStruct() || declVar() || declFunc())
        ;
    if (!consume(END))
    {
        tkerr(crtTk, "Expected END");
    }
    return 1;
}

int declStruct()
{
    Token *startTk = crtTk;
    if (!consume(STRUCT))
    {
        return 0;
    }
    if (!consume(ID))
    {
        tkerr(crtTk, "Missing identifier after struct");
    }

    char *tkName = consumedTk->text;

    if (!consume(LACC))
    {
        crtTk = startTk;
        return 0;
    }
    if (findSymbol(getSymbolsTable(), tkName))
        tkerr(crtTk, "symbol redefinition: %s", tkName);
    setCrtStruct(addSymbol(getSymbolsTable(), tkName, CLS_STRUCT));
    initSymbols(&getCrtStruct()->members);

    while (declVar())
        ;
    if (!consume(RACC))
    {
        tkerr(crtTk, "Missing }");
    }
    if (!consume(SEMICOLON))
    {
        tkerr(crtTk, "Missing ; after struct");
    }

    resetCrtStruct();

    return 1;
}

int typeBase();
int arrayDecl();

int declVar()
{
    Token *startTk = crtTk;
    Symbol *afterSymbol = getLastSymbol(getSymbolsTable());
    Type *t;
    SAFEALLOC(t, Type);
    if (!typeBase(t))
    {
        free(t);
        return 0;
    }
    if (!consume(ID))
    {
        free(t);
        crtTk = startTk;
        return 0;
    }

    Token *tkName = consumedTk;

    int isDeclVar = 0;
    if (arrayDecl(t))
    {
        isDeclVar = 1;
    }

    else
    {
        t->nElements = -1;
    }
    addVar(tkName, t);

    while (consume(COMMA))
    {
        isDeclVar = 1;
        if (!consume(ID))
        {
            tkerr(crtTk, "Missing identifier after ,");
        }

        tkName = consumedTk;

        if (!arrayDecl(t))
        {
            t->nElements = -1;
        }

        addVar(tkName, t);
    }
    if (!consume(SEMICOLON))
    {
        if (isDeclVar)
        {
            tkerr(crtTk, "Missing ; after declaration");
        }
        else
        {
            free(t);
            deleteSymbolsAfter(getSymbolsTable(), afterSymbol);
            crtTk = startTk;
            return 0;
        }
    }
    free(t);
    return 1;
}

int typeBase(Type *ret)
{
    if (consume(INT))
    {
        ret->typeBase = TB_INT;
        return 1;
    }
    if (consume(DOUBLE))
    {
        ret->typeBase = TB_DOUBLE;
        return 1;
    }
    if (consume(CHAR))
    {
        ret->typeBase = TB_CHAR;
        return 1;
    }
    if (consume(STRUCT))
    {
        if (!consume(ID))
        {
            tkerr(crtTk, "Missing identifier after struct");
        }

        Symbol *s = findSymbol(getSymbolsTable(), consumedTk->text);
        if (s == NULL)
        {
            tkerr(crtTk, "Undefined symbol: %s", consumedTk->text);
        }
        if (s->cls != CLS_STRUCT)
        {
            tkerr(crtTk, "%s is not a struct", consumedTk->text);
        }
        ret->typeBase = TB_STRUCT;
        ret->s = s;

        return 1;
    }
    return 0;
}

int expr();

int arrayDecl(Type *ret)
{
    if (!consume(LBRACKET))
    {
        return 0;
    }

    RetVal rv;

    if (expr(&rv))
    {
        if (!rv.isCtVal)
        {
            tkerr(crtTk, "the array size is not a constant");
        }
        if (rv.type.typeBase != TB_INT)
        {
            tkerr(crtTk, "the array size is not an integer");
        }
        ret->nElements = rv.ctVal.i;
    }
    else
    {
        ret->nElements = 0;
    }

    if (!consume(RBRACKET))
    {
        tkerr(crtTk, "Missing ]");
    }
    return 1;
}

int typeName(Type *ret)
{
    if (!typeBase(ret))
    {
        return 0;
    }

    if (!arrayDecl(ret))
    {
        ret->nElements = -1;
    }

    return 1;
}

int funcArg();
int stmCompound();

int declFunc()
{
    Type *t;
    SAFEALLOC(t, Type);
    if (typeBase(t))
    {
        if (consume(MUL))
        {
            t->nElements = 0;
        }
        else
        {
            t->nElements = -1;
        }
    }
    else if (consume(VOID))
    {
        t->typeBase = TB_VOID;
    }
    else
    {
        free(t);
        return 0;
    }

    if (!consume(ID))
    {
        tkerr(crtTk, "Missing identifier after function type");
    }

    char *tkName = consumedTk->text;

    if (!consume(LPAR))
    {
        tkerr(crtTk, "Missing ( after function name");
    }

    if (findSymbol(getSymbolsTable(), tkName))
        tkerr(crtTk, "symbol redefinition: %s", tkName);
    setCrtFunc(addSymbol(getSymbolsTable(), tkName, CLS_FUNC));
    initSymbols(&getCrtFunc()->args);
    getCrtFunc()->type = *t;
    setCrtDepth(getCrtDepth() + 1);

    if (funcArg())
    {
        while (consume(COMMA))
        {
            if (!funcArg())
            {
                tkerr(crtTk, "Missing function argument after ,");
            }
        }
    }
    if (!consume(RPAR))
    {
        tkerr(crtTk, "Missing )");
    }

    setCrtDepth(getCrtDepth() - 1);

    if (!stmCompound())
    {
        tkerr(crtTk, "Missing compound statement");
    }

    deleteSymbolsAfter(getSymbolsTable(), getCrtFunc());
    setCrtFunc(NULL);

    free(t);
    return 1;
}

int funcArg()
{
    Type *t;
    SAFEALLOC(t, Type);
    if (!typeBase(t))
    {
        free(t);
        return 0;
    }
    if (!consume(ID))
    {
        tkerr(crtTk, "Missing identifier after function argument type");
    }

    char *tkName = consumedTk->text;

    if (!arrayDecl(t))
    {
        t->nElements = -1;
    }
    Symbol *s = addSymbol(getSymbolsTable(), tkName, CLS_VAR);
    s->mem = MEM_ARG;
    s->type = *t;
    s = addSymbol(&getCrtFunc()->args, tkName, CLS_VAR);
    s->mem = MEM_ARG;
    s->type = *t;

    free(t);
    return 1;
}

int expr();

int stm()
{
    if (stmCompound())
    {
        return 1;
    }

    RetVal rv;

    if (consume(IF))
    {
        if (!consume(LPAR))
        {
            tkerr(crtTk, "Missing ( after if");
        }
        if (!expr(&rv))
        {
            tkerr(crtTk, "Missing expression after (");
        }

        if (rv.type.typeBase == TB_STRUCT)
        {
            tkerr(crtTk, "a structure cannot be logically tested");
        }

        if (!consume(RPAR))
        {
            tkerr(crtTk, "Missing )");
        }
        if (!stm())
        {
            tkerr(crtTk, "Missing statement after if");
        }
        if (consume(ELSE))
        {
            if (!stm())
            {
                tkerr(crtTk, "Missing statement after else");
            }
        }
        return 1;
    }
    if (consume(WHILE))
    {
        if (!consume(LPAR))
        {
            tkerr(crtTk, "Missing ( after while");
        }
        if (!expr(&rv))
        {
            tkerr(crtTk, "Missing expression after (");
        }

        if (rv.type.typeBase == TB_STRUCT)
        {
            tkerr(crtTk, "a structure cannot be logically tested");
        }

        if (!consume(RPAR))
        {
            tkerr(crtTk, "Missing )");
        }
        if (!stm())
        {
            tkerr(crtTk, "Missing statement after while");
        }
        return 1;
    }
    if (consume(FOR))
    {
        if (!consume(LPAR))
        {
            tkerr(crtTk, "Missing ( after for");
        }

        RetVal rv1;

        expr(&rv1);
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after expression");
        }

        RetVal rv2;
        if (expr(&rv2))
        {
            if (rv2.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be logically tested");
            }
        }

        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after expression");
        }

        RetVal rv3;

        expr(&rv3);
        if (!consume(RPAR))
        {
            tkerr(crtTk, "Missing )");
        }
        if (!stm())
        {
            tkerr(crtTk, "Missing statement after for");
        }
        return 1;
    }
    if (consume(BREAK))
    {
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after break");
        }
        return 1;
    }
    if (consume(RETURN))
    {
        if (expr(&rv))
        {
            if (getCrtFunc()->type.typeBase == TB_VOID)
            {
                tkerr(crtTk, "a void function cannot return a value");
            }
            cast(&getCrtFunc()->type, &rv.type);
        }

        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after return");
        }
        return 1;
    }
    if (expr(&rv))
    {
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after expression");
        }
        return 1;
    }
    else
    {
        if (consume(SEMICOLON))
        {
            return 1;
        }
        return 0;
    }
}

int stmCompound()
{
    Symbol *afterSymbol = getLastSymbol(getSymbolsTable());
    if (!consume(LACC))
    {
        return 0;
    }
    setCrtDepth(getCrtDepth() + 1);
    while (declVar() || stm())
        ;
    if (!consume(RACC))
    {
        tkerr(crtTk, "Missing }");
    }

    setCrtDepth(getCrtDepth() - 1);
    deleteSymbolsAfter(getSymbolsTable(), afterSymbol);
    return 1;
}

int expr();

int exprPrimary(RetVal *rv)
{
    if (consume(CT_INT) || consume(CT_REAL) || consume(CT_CHAR) || consume(CT_STRING))
    {
        if (consumedTk->code == CT_INT)
        {
            rv->type = createType(TB_INT, -1);
            rv->ctVal.i = consumedTk->i;
        }
        else if (consumedTk->code == CT_REAL)
        {
            rv->type = createType(TB_DOUBLE, -1);
            rv->ctVal.d = consumedTk->r;
        }
        else if (consumedTk->code == CT_CHAR)
        {
            rv->type = createType(TB_CHAR, -1);
            rv->ctVal.i = consumedTk->i;
        }
        else if (consumedTk->code == CT_STRING)
        {
            rv->type = createType(TB_CHAR, 0);
            rv->ctVal.str = consumedTk->text;
        }
        rv->isCtVal = 1;
        rv->isLVal = 0;
        return 1;
    }
    if (consume(ID))
    {
        Token *tkName = consumedTk;
        Symbol *s = findSymbol(getSymbolsTable(), tkName->text);
        if (!s)
        {
            tkerr(crtTk, "Undefined symbol: %s", tkName->text);
        }
        rv->type = s->type;
        rv->isCtVal = 0;
        rv->isLVal = 1;

        if (consume(LPAR))
        {
            Symbol **crtDefArg = s->args.begin;
            if (s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
            {
                tkerr(crtTk, "call of the non-function %s", tkName->text);
            }

            RetVal arg;
            if (expr(&arg))
            {
                if (crtDefArg == s->args.end)
                {
                    tkerr(crtTk, "too many arguments in call");
                }
                cast(&(*crtDefArg)->type, &arg.type);
                crtDefArg++;

                while (consume(COMMA))
                {
                    if (!expr(&arg))
                    {
                        if (crtDefArg == s->args.end)
                        {
                            tkerr(crtTk, "too many arguments in call");
                        }
                        cast(&(*crtDefArg)->type, &arg.type);
                        crtDefArg++;

                        tkerr(crtTk, "Missing expression after ,");
                    }
                }
            }
            if (!consume(RPAR))
            {
                tkerr(crtTk, "Missing )");
            }
            if (crtDefArg != s->args.end)
            {
                tkerr(crtTk, "too few arguments in call");
            }
            rv->type = s->type;
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }

        else if (s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
        {
            tkerr(crtTk, "missing call for function %s", tkName->text);
        }
        return 1;
    }
    if (consume(LPAR))
    {
        if (!expr(rv))
        {
            tkerr(crtTk, "Missing expression after (");
        }
        if (!consume(RPAR))
        {
            tkerr(crtTk, "Missing )");
        }
        return 1;
    }
    return 0;
}

int exprPostfix(RetVal *rv)
{
    if (exprPrimary(rv))
    {
        while (consume(DOT) || consume(LBRACKET))
        {
            if (getTokenCode(consumedTk) == DOT)
            {
                if (!consume(ID))
                {
                    tkerr(crtTk, "Missing identifier after .");
                }

                Symbol *sStruct = rv->type.s;
                Symbol *sMember = findSymbol(&sStruct->members, consumedTk->text);
                if (!sMember)
                {
                    tkerr(crtTk, "struct %s does not have a member %s", sStruct->name, consumedTk->text);
                }
                rv->type = sMember->type;
                rv->isLVal = 1;
                rv->isCtVal = 0;
            }
            else
            {
                RetVal rve;
                if (!expr(&rve))
                {
                    tkerr(crtTk, "Missing expression after [");
                }

                if (rv->type.nElements < 0)
                {
                    tkerr(crtTk, "only an array can be indexed");
                }
                Type typeInt = createType(TB_INT, -1);
                cast(&typeInt, &rve.type);
                rv->type = rv->type;
                rv->type.nElements = -1;
                rv->isLVal = 1;

                rv->isCtVal = 0;

                if (!consume(RBRACKET))
                {
                    tkerr(crtTk, "Missing ]");
                }
            }
        }
        return 1;
    }
    return 0;
}

int exprUnary(RetVal *rv)
{
    if (exprPostfix(rv))
    {
        return 1;
    }
    if (consume(SUB) || consume(NOT))
    {
        Token *tkOp = consumedTk;

        if (!exprUnary(rv))
        {
            tkerr(crtTk, "Missing expression after unary operator");
        }

        if (tkOp->code == SUB)
        {
            if (rv->type.nElements >= 0)
            {
                tkerr(crtTk, "unary '-' cannot be applied to an array");
            }
            if (rv->type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "unary '-' cannot be applied to a struct");
            }
        }
        else
        { // NOT
            if (rv->type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "'!' cannot be applied to a struct");
            }
            rv->type = createType(TB_INT, -1);
        }
        rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        return 1;
    }
    return 0;
}

int exprCast(RetVal *rv)
{
    if (exprUnary(rv))
    {
        return 1;
    }
    if (consume(LPAR))
    {
        Type *t;
        SAFEALLOC(t, Type);
        if (!typeName(t))
        {
            tkerr(crtTk, "Missing type name after (");
        }
        if (!consume(RPAR))
        {
            tkerr(crtTk, "Missing )");
        }

        RetVal rve;

        if (!exprCast(&rve))
        {
            tkerr(crtTk, "Missing expression after cast");
        }

        cast(t, &rve.type);
        rv->type = *t;
        rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity

        return 1;
    }
    return 0;
}

int exprMul(RetVal *rv)
{
    if (exprCast(rv))
    {
        while (consume(MUL) || consume(DIV))
        {
            // Token *tkOp = consumedTk;
            RetVal rve;

            if (!exprCast(&rve))
            {
                tkerr(crtTk, "Missing expression after * / or %");
            }

            if (rv->type.nElements > -1 || rve.type.nElements > -1)
            {
                tkerr(crtTk, "an array cannot be multiplied or divided");
            }
            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be multiplied or divided");
            }
            rv->type = getArithType(&rv->type, &rve.type);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprAdd(RetVal *rv)
{
    if (exprMul(rv))
    {
        while (consume(ADD) || consume(SUB))
        {
            // Token *tkOp = consumedTk;
            RetVal rve;

            if (!exprMul(&rve))
            {
                tkerr(crtTk, "Missing expression after + or -");
            }

            if (rv->type.nElements > -1 || rve.type.nElements > -1)
            {
                tkerr(crtTk, "an array cannot be added or subtracted");
            }
            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be added or subtracted");
            }
            rv->type = getArithType(&rv->type, &rve.type);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprRel(RetVal *rv)
{
    if (exprAdd(rv))
    {
        while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
        {
            // Token *tkOp = consumedTk;
            RetVal rve;

            if (!exprAdd(&rve))
            {
                tkerr(crtTk, "Missing expression after relational operator");
            }

            if (rv->type.nElements > -1 || rve.type.nElements > -1)
            {
                tkerr(crtTk, "an array cannot be compared");
            }
            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be compared");
            }
            rv->type = createType(TB_INT, -1);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprEq(RetVal *rv)
{
    if (exprRel(rv))
    {
        while (consume(EQUAL) || consume(NOTEQ))
        {
            // Token *tkOp = consumedTk;
            RetVal rve;

            if (!exprRel(&rve))
            {
                tkerr(crtTk, "Missing expression after == or !=");
            }

            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be compared");
            }
            rv->type = createType(TB_INT, -1);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprAnd(RetVal *rv)
{
    if (exprEq(rv))
    {
        while (consume(AND))
        {
            RetVal rve;

            if (!exprEq(&rve))
            {
                tkerr(crtTk, "Missing expression after &&");
            }

            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be logically tested");
            }
            rv->type = createType(TB_INT, -1);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprOr(RetVal *rv)
{
    if (exprAnd(rv))
    {
        while (consume(OR))
        {
            RetVal rve;

            if (!exprAnd(&rve))
            {
                tkerr(crtTk, "Missing expression after ||");
            }

            if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
            {
                tkerr(crtTk, "a structure cannot be logically tested");
            }
            rv->type = createType(TB_INT, -1);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity
        }
        return 1;
    }
    return 0;
}

int exprAssign(RetVal *rv)
{
    Token *startTk = crtTk;
    if (exprUnary(rv))
    {
        if (consume(ASSIGN))
        {
            RetVal rve;

            if (!exprAssign(&rve))
            {
                tkerr(crtTk, "Missing expression after =");
            }

            if (!rv->isLVal)
            {
                tkerr(crtTk, "cannot assign to a non-lval");
            }
            if (rv->type.nElements > -1 || rve.type.nElements > -1)
            {
                tkerr(crtTk, "the arrays cannot be assigned");
            }
            cast(&rv->type, &rve.type);
            rv->isCtVal = rv->isLVal = 0; // compromise to preserve sanity

            return 1;
        }
    }
    crtTk = startTk;
    if (exprOr(rv))
    {
        return 1;
    }
    return 0;
}

int expr(RetVal *rv)
{
    if (exprAssign(rv))
    {
        return 1;
    }
    return 0;
}

void checkSyntax(Token *tokens)
{
    crtTk = tokens;

    addExtFuncs();
    if (unit())
    {
        printf("Syntax OK\n");
    }
    else
    {
        printf("Syntax NOT OK\n");
    }
}