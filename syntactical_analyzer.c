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
    expr();

    ret->nElements = 0;

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
    if (consume(IF))
    {
        if (!consume(LPAR))
        {
            tkerr(crtTk, "Missing ( after if");
        }
        if (!expr())
        {
            tkerr(crtTk, "Missing expression after (");
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
        if (!expr())
        {
            tkerr(crtTk, "Missing expression after (");
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
        expr();
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after expression");
        }
        expr();
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after expression");
        }
        expr();
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
        expr();
        if (!consume(SEMICOLON))
        {
            tkerr(crtTk, "Missing ; after return");
        }
        return 1;
    }
    if (expr())
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

int exprPrimary()
{
    if (consume(CT_INT) || consume(CT_REAL) || consume(CT_CHAR) || consume(CT_STRING))
    {
        return 1;
    }
    if (consume(ID))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                while (consume(COMMA))
                {
                    if (!expr())
                    {
                        tkerr(crtTk, "Missing expression after ,");
                    }
                }
            }
            if (!consume(RPAR))
            {
                tkerr(crtTk, "Missing )");
            }
        }
        return 1;
    }
    if (consume(LPAR))
    {
        if (!expr())
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

int exprPostfix()
{
    if (exprPrimary())
    {
        while (consume(DOT) || consume(LBRACKET))
        {
            if (getTokenCode(consumedTk) == DOT)
            {
                if (!consume(ID))
                {
                    tkerr(crtTk, "Missing identifier after .");
                }
            }
            else
            {
                if (!expr())
                {
                    tkerr(crtTk, "Missing expression after [");
                }
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

int exprUnary()
{
    if (exprPostfix())
    {
        return 1;
    }
    if (consume(SUB) || consume(NOT))
    {
        if (!exprUnary())
        {
            tkerr(crtTk, "Missing expression after unary operator");
        }
        return 1;
    }
    return 0;
}

int exprCast()
{
    if (exprUnary())
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
        if (!exprCast())
        {
            tkerr(crtTk, "Missing expression after cast");
        }
        return 1;
    }
    return 0;
}

int exprMul()
{
    if (exprCast())
    {
        while (consume(MUL) || consume(DIV))
        {
            if (!exprCast())
            {
                tkerr(crtTk, "Missing expression after * / or %");
            }
        }
        return 1;
    }
    return 0;
}

int exprAdd()
{
    if (exprMul())
    {
        while (consume(ADD) || consume(SUB))
        {
            if (!exprMul())
            {
                tkerr(crtTk, "Missing expression after + or -");
            }
        }
        return 1;
    }
    return 0;
}

int exprRel()
{
    if (exprAdd())
    {
        while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
        {
            if (!exprAdd())
            {
                tkerr(crtTk, "Missing expression after relational operator");
            }
        }
        return 1;
    }
    return 0;
}

int exprEq()
{
    if (exprRel())
    {
        while (consume(EQUAL) || consume(NOTEQ))
        {
            if (!exprRel())
            {
                tkerr(crtTk, "Missing expression after == or !=");
            }
        }
        return 1;
    }
    return 0;
}

int exprAnd()
{
    if (exprEq())
    {
        while (consume(AND))
        {
            if (!exprEq())
            {
                tkerr(crtTk, "Missing expression after &&");
            }
        }
        return 1;
    }
    return 0;
}

int exprOr()
{
    if (exprAnd())
    {
        while (consume(OR))
        {
            if (!exprAnd())
            {
                tkerr(crtTk, "Missing expression after ||");
            }
        }
        return 1;
    }
    return 0;
}

int exprAssign()
{
    Token *startTk = crtTk;
    if (exprUnary())
    {
        if (consume(ASSIGN))
        {
            if (!exprAssign())
            {
                tkerr(crtTk, "Missing expression after =");
            }
            return 1;
        }
    }
    crtTk = startTk;
    if (exprOr())
    {
        return 1;
    }
    return 0;
}

int expr()
{
    if (exprAssign())
    {
        return 1;
    }
    return 0;
}

void checkSyntax(Token *tokens)
{
    crtTk = tokens;
    if (unit())
    {
        printf("Syntax OK\n");
    }
    else
    {
        printf("Syntax NOT OK\n");
    }
}