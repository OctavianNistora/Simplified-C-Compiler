#ifndef SEMANTIC_ANALYZER_FRAGMENT_H_INCLUDED
#define SEMANTIC_ANALYZER_FRAGMENT_H_INCLUDED
#include "types.h"

Symbols *getSymbolsTable();

int getCrtDepth();

void setCrtDepth(int depth);

Symbol *getCrtFunc();

void setCrtFunc(Symbol *s);

void resetCrtFunc();

Symbol *getCrtStruct();

void setCrtStruct(Symbol *s);

void resetCrtStruct();

void initSymbols(Symbols *symbols);

Symbol *addSymbol(Symbols *symbols, const char *name, int cls);

void addVar(Token *tkName, Type *t);

Symbol *findSymbol(Symbols *symbols, const char *name);

Symbol *getLastSymbol(Symbols *symbols);

void deleteSymbolsAfter(Symbols *symbols, Symbol *after);

void emptySymbols(Symbols *symbols);

Type createType(int typeBase, int nElements);

void cast(Type *dst, Type *src);

Symbol *addExtFunc(const char *name, Type type);

Symbol *addFuncArg(Symbol *func, const char *name, Type type);

void addExtFuncs();

Type getArithType(Type *s1, Type *s2);

#endif