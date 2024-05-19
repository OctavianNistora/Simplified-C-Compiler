#include <stdio.h>
#include <stdlib.h>
#include "lexical_analyzer.h"
#include "semantic_analyzer_fragment.h"
#include "syntactical_analyzer.h"
#include "virtual_machine.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 2;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    setPCrtCh(malloc(length));
    if (getPCrtCh())
    {
        fread(getPCrtCh(), 1, length, f);
    }
    fclose(f);

    while (getNextToken() != END)
        ;
    freePcrtCh();

    Token *token = getTokens();

    /*for (Token *current = token; current != NULL; current = getTokenNext(current))
    {
        printf("%d", getTokenCode(current));
        if (getTokenCode(current) == ID)
        {
            printf(":%s", getTokenText(current));
        }
        else if (getTokenCode(current) == CT_INT)
        {
            printf(":%ld", getTokenI(current));
        }
        else if (getTokenCode(current) == CT_REAL)
        {
            printf(":%f", getTokenR(current));
        }
        else if (getTokenCode(current) == CT_CHAR)
        {
            printf(":%c", (char)getTokenI(current));
        }
        else if (getTokenCode(current) == CT_STRING)
        {
            printf(":%s", getTokenText(current));
        }
        printf(" ");
    }
    printf("\n");*/
    initSymbols(getSymbolsTable());
    checkSyntax(token);

    execute();

    emptySymbols(getSymbolsTable());
    freeTokens();
    return 0;
}