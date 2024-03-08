#include <stdio.h>
#include <stdlib.h>
#include "lexical_analyzer.h"

int main()
{
    FILE *f = fopen("5.c", "r");
    long length;
    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        setPCrtCh(malloc(length));
        if (getPCrtCh())
        {
            fread(getPCrtCh(), 1, length, f);
        }
        fclose(f);
    }
    while (getNextToken() != END)
        ;
    Token *current = getTokens();
    while (current != NULL)
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
        current = getTokenNext(current);
    }
    printf("\n");
    return 0;
}