#include <stdio.h>
#include <string.h>
#include "types.h"
#include "defines.h"
#include "syntactical_analyzer.h"
#include "semantic_analyzer_fragment.h"
#include "external_functions.h"
#include "virtual_machine.h"

Symbols symbols;
int crtDepth = 0;
Symbol *crtFunc;
Symbol *crtStruct;
int sizeArgs, offset;

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
    offset = 0;
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
    offset = 0;
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

    if (crtStruct || crtFunc)
    {
        s->offset = offset;
    }
    else
    {
        s->addr = allocGlobal(typeFullSize(&s->type));
    }
    offset += typeFullSize(&s->type);
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

Type createType(int typeBase, int nElements)
{
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;
    return t;
}

void cast(Type *dst, Type *src)
{
    if (src->nElements > -1)
    {
        if (dst->nElements > -1)
        {
            if (src->typeBase != dst->typeBase)
                tkerr(getCrtTk(), "an array cannot be converted to an array of another type");
        }
        else
        {
            tkerr(getCrtTk(), "an array cannot be converted to a non-array");
        }
    }
    else
    {
        if (dst->nElements > -1)
        {
            tkerr(getCrtTk(), "a non-array cannot be converted to an array");
        }
    }
    switch (src->typeBase)
    {
    case TB_CHAR:
    case TB_INT:
    case TB_DOUBLE:
        switch (dst->typeBase)
        {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            return;
        }
    case TB_STRUCT:
        if (dst->typeBase == TB_STRUCT)
        {
            if (src->s != dst->s)
                tkerr(getCrtTk(), "a structure cannot be converted to another one");
            return;
        }
    }
    tkerr(getCrtTk(), "incompatible types");
}

Symbol *addExtFunc(const char *name, Type type, void *addr)
{
    Symbol *s = addSymbol(&symbols, name, CLS_EXTFUNC);
    s->type = type;
    s->addr = addr;
    initSymbols(&s->args);
    return s;
}

Symbol *addFuncArg(Symbol *func, const char *name, Type type)
{
    Symbol *a = addSymbol(&func->args, name, CLS_VAR);
    a->type = type;
    return a;
}

void addExtFuncs()
{
    Symbol *s;
    s = addExtFunc("put_s", createType(TB_VOID, -1), put_s);
    addFuncArg(s, "s", createType(TB_CHAR, 0));

    s = addExtFunc("get_s", createType(TB_VOID, -1), get_s);
    addFuncArg(s, "s", createType(TB_CHAR, 0));

    s = addExtFunc("put_i", createType(TB_VOID, -1), put_i);
    addFuncArg(s, "i", createType(TB_INT, -1));

    s = addExtFunc("get_i", createType(TB_INT, -1), get_i);

    s = addExtFunc("put_d", createType(TB_VOID, -1), put_d);
    addFuncArg(s, "d", createType(TB_DOUBLE, -1));

    s = addExtFunc("get_d", createType(TB_DOUBLE, -1), get_d);

    s = addExtFunc("put_c", createType(TB_VOID, -1), put_c);
    addFuncArg(s, "c", createType(TB_CHAR, -1));

    s = addExtFunc("get_c", createType(TB_CHAR, -1), get_c);

    s = addExtFunc("seconds", createType(TB_DOUBLE, -1), seconds);
}

Type getArithType(Type *s1, Type *s2)
{
    if (s1->typeBase == TB_DOUBLE || s2->typeBase == TB_DOUBLE)
    {
        return createType(TB_DOUBLE, -1);
    }
    if (s1->typeBase == TB_INT || s2->typeBase == TB_INT)
    {
        return createType(TB_INT, -1);
    }
    return createType(TB_CHAR, -1);
}

Symbol *requireSymbol(Symbols *symbols, const char *name)
{
    Symbol *s = findSymbol(symbols, name);
    if (!s)
    {
        tkerr(getCrtTk(), "undefined symbol: %s", name);
    }
    return s;
}

int getSizeArgs()
{
    return sizeArgs;
}

void setSizeArgs(int size)
{
    sizeArgs = size;
}

int getOffset()
{
    return offset;
}

void setOffset(int off)
{
    offset = off;
}