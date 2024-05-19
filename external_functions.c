#include <stdio.h>
#include <string.h>
#include <time.h>
#include "defines.h"
#include "stack_fragment.h"
#include "external_functions.h"

void put_s()
{
    printf("%s", (char *)popa());
}

void get_s()
{
    char *s;
    SAFESTRALLOC(s, 4096);
    scanf("%s", s);
    SAFEREALLOC(s, char, strlen(s) + 1);
    pusha(s);
}

void put_i()
{
    printf("%ld", popi());
}

void get_i()
{
    long i;
    scanf("%ld", &i);
    pushi(i);
}

void put_d()
{
    printf("%lf", popd());
}

void get_d()
{
    double d;
    scanf("%lf", &d);
    pushd(d);
}

void put_c()
{
    printf("%c", popc());
}

void get_c()
{
    char c;
    scanf("%c", &c);
    pushc(c);
}

void seconds()
{
    time_t t = time(NULL);
    pushi(t);
}