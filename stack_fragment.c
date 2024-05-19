#include <stdio.h>
#include "defines.h"
#include "lexical_analyzer.h"

char stack[STACK_SIZE];
char *SP;         // Stack Pointer
char *stackAfter; // first byte after stack; used for stack limit tests

char *getStack()
{
    return stack;
}

char *getSP()
{
    return SP;
}

void setSP(char *sp)
{
    SP = sp;
}

char *getStackAfter()
{
    return stackAfter;
}

void setStackAfter(char *stackafter)
{
    stackAfter = stackafter;
}

void pusha(void *a)
{
    if (SP + sizeof(void *) > stackAfter)
    {
        err("out of stack");
    }
    *(void **)SP = a;
    SP += sizeof(void *);
}

void *popa()
{
    SP -= sizeof(void *);
    if (SP < stack)
    {
        err("not enough stack bytes for popa");
    }
    return *(void **)SP;
}

void pushc(char c)
{
    if (SP + sizeof(char) > stackAfter)
    {
        err("out of stack");
    }
    *(char *)SP = c;
    SP += sizeof(char);
}

char popc()
{
    SP -= sizeof(char);
    if (SP < stack)
    {
        err("not enough stack bytes for popc");
    }
    return *(char *)SP;
}

void pushd(double d)
{
    if (SP + sizeof(double) > stackAfter)
    {
        err("out of stack");
    }
    *(double *)SP = d;
    SP += sizeof(double);
}

double popd()
{
    SP -= sizeof(double);
    if (SP < stack)
    {
        err("not enough stack bytes for popd");
    }
    return *(double *)SP;
}

void pushi(long int i)
{
    if (SP + sizeof(long int) > stackAfter)
    {
        err("out of stack");
    }
    *(long int *)SP = i;
    SP += sizeof(long int);
}

long int popi()
{
    SP -= sizeof(long int);
    if (SP < stack)
    {
        err("not enough stack bytes for popi");
    }
    return *(long int *)SP;
}