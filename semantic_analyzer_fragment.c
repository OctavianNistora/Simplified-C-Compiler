#include <stdio.h>
#include <string.h>
#include "types.h"
#include "defines.h"
#include "syntactical_analyzer.h"
#include "semantic_analyzer_fragment.h"

Symbols symbols;
int crtDepth = 0;
Symbol *crtFunc;
Symbol *crtStruct;

Symbols *getSymbolsTable()
{
    return &symbols;
}

int getCrtDepth()
{
    return crtDepth;
}

void setCrtDepth(int depth)
{
    crtDepth = depth;
}

Symbol *getCrtFunc()
{
    return crtFunc;
}

void setCrtFunc(Symbol *s)
{
    crtFunc = s;
}

void resetCrtFunc()
{
    crtFunc = NULL;
}

Symbol *getCrtStruct()
{
    return crtStruct;
}

void setCrtStruct(Symbol *s)
{
    crtStruct = s;
}

void resetCrtStruct()
{
    crtStruct = NULL;
}

void initSymbols(Symbols *symbols)
{
    symbols->begin = NULL;
    symbols->end = NULL;
    symbols->after = NULL;
}

Symbol *addSymbol(Symbols *symbols, const char *name, int cls)
{
    Symbol *s;
    if (symbols->end == symbols->after)
    { // create more room
        int count = symbols->after - symbols->begin;
        int n = count * 2; // double the room
        if (n == 0)
            n = 1; // needed for the initial case
        SAFEREALLOC(symbols->begin, Symbol *, n + 2);
        symbols->end = symbols->begin + count;
        symbols->after = symbols->begin + n;
    }
    SAFEALLOC(s, Symbol)
    *(symbols->end++) = s;
    s->name = name;
    s->cls = cls;
    s->depth = crtDepth;
    return s;
}

void addVar(Token *tkName, Type *t)
{
    Symbol *s;
    if (crtStruct)
    {
        if (findSymbol(&crtStruct->members, tkName->text))
            tkerr(getCrtTk(), "symbol redefinition: %s", tkName->text);
        s = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
    }
    else if (crtFunc)
    {
        s = findSymbol(&symbols, tkName->text);
        if (s && s->depth == crtDepth)
            tkerr(getCrtTk(), "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_LOCAL;
    }
    else
    {
        if (findSymbol(&symbols, tkName->text))
            tkerr(getCrtTk(), "symbol redefinition: %s", tkName->text);
        s = addSymbol(&symbols, tkName->text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }
    s->type = *t;
}

Symbol *findSymbol(Symbols *symbols, const char *name)
{
    // printf("HERE");
    if (symbols->begin == NULL)
        return NULL;
    for (Symbol **s = symbols->end - 1; s >= symbols->begin; s--)
    {
        if (strcmp((*s)->name, name) == 0)
        {
            return *s;
        }
    }
    return NULL;
}

Symbol *getLastSymbol(Symbols *symbols)
{
    if (symbols->begin == NULL)
        return NULL;
    return symbols->end[-1];
}

void deleteSymbolsAfter(Symbols *symbols, Symbol *after)
{
    if (after == NULL)
    {
        for (Symbol **s = symbols->end - 1; s >= symbols->begin; s--)
        {
            Symbol *temp = *s;
            free(temp);
        }
        free(symbols->begin);
        symbols->begin = NULL;
        symbols->end = NULL;
        symbols->after = NULL;
        return;
    }
    for (Symbol **s = symbols->end - 1; s >= symbols->begin; s--)
    {
        if (*s == after)
        {
            s++;
            Symbol **auxend = s;
            while (s != symbols->end)
            {
                Symbol *temp = *s;
                free(temp);
                s++;
            }
            symbols->end = auxend;
            return;
        }
    }
}

void emptySymbols(Symbols *symbols)
{
    if (symbols->begin == NULL)
        return;
    for (Symbol **s = symbols->end - 1; s >= symbols->begin; s--)
    {
        Symbol *temp = *s;
        if (temp->cls == CLS_FUNC)
        {
            emptySymbols(&temp->args);
        }
        else if (temp->cls == CLS_STRUCT)
        {
            emptySymbols(&temp->members);
        }
        free(temp);
    }
    free(symbols->begin);
    symbols->begin = NULL;
    symbols->end = NULL;
    symbols->after = NULL;
}