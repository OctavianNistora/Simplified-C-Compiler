#ifndef LEXICAL_ANALYZER_H_INCLUDED
#define LEXICAL_ANALYZER_H_INCLUDED

#include "types.h"

void err(const char *fmt, ...);

void tkerr(const Token *tk, const char *fmt, ...);

int getNextToken();

int getTokenCode(Token *tk);

char *getTokenText(Token *tk);

long int getTokenI(Token *tk);

double getTokenR(Token *tk);

Token *getTokenNext(Token *tk);

char *getPCrtCh();

void setPCrtCh(char *new);

void freePcrtCh();

Token *getTokens();

void freeTokens();

#endif