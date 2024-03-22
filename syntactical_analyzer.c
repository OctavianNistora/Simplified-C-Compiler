#include <stdio.h>
#include <stdlib.h>
#include "lexical_analyzer.h"

Token *crtTk;
Token *consumedTk;

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
    if (!consume(LACC))
    {
        crtTk = startTk;
        return 0;
    }
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
    return 1;
}

int typeBase();
int arrayDecl();

int declVar()
{
    Token *startTk = crtTk;
    if (!typeBase())
    {
        return 0;
    }
    if (!consume(ID))
    {
        crtTk = startTk;
        return 0;
    }
    int isDeclVar = 0;
    if (arrayDecl())
    {
        isDeclVar = 1;
    }
    while (consume(COMMA))
    {
        isDeclVar = 1;
        if (!consume(ID))
        {
            tkerr(crtTk, "Missing identifier after ,");
        }
        arrayDecl();
    }
    if (!consume(SEMICOLON))
    {
        if (isDeclVar)
        {
            tkerr(crtTk, "Missing ; after declaration");
        }
        else
        {
            crtTk = startTk;
            return 0;
        }
    }
    return 1;
}

int typeBase()
{
    if (consume(INT) || consume(DOUBLE) || consume(CHAR))
    {
        return 1;
    }
    if (consume(STRUCT))
    {
        if (!consume(ID))
        {
            tkerr(crtTk, "Missing identifier after struct");
        }
        return 1;
    }
    return 0;
}

int expr();

int arrayDecl()
{
    if (!consume(LBRACKET))
    {
        return 0;
    }
    expr();
    if (!consume(RBRACKET))
    {
        tkerr(crtTk, "Missing ]");
    }
    return 1;
}

int typeName()
{
    if (!typeBase())
    {
        return 0;
    }
    arrayDecl();
    return 1;
}

int funcArg();
int stmCompound();

int declFunc()
{
    if (typeBase())
    {
        consume(MUL);
    }
    else if (!consume(VOID))
    {
        return 0;
    }
    if (!consume(ID))
    {
        tkerr(crtTk, "Missing identifier after function type");
    }
    if (!consume(LPAR))
    {
        tkerr(crtTk, "Missing ( after function name");
    }
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
    if (!stmCompound())
    {
        tkerr(crtTk, "Missing compound statement");
    }
    return 1;
}

int funcArg()
{
    if (!typeBase())
    {
        return 0;
    }
    if (!consume(ID))
    {
        tkerr(crtTk, "Missing identifier after function argument type");
    }
    arrayDecl();
    return 1;
}

int stmCompound();
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
        if (!consume(SEMICOLON))
        {
            return 0;
        }
        return 1;
    }
}

int stmCompound()
{
    if (!consume(LACC))
    {
        return 0;
    }
    while (declVar() || stm())
        ;
    if (!consume(RACC))
    {
        tkerr(crtTk, "Missing }");
    }
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
        if (!typeName())
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