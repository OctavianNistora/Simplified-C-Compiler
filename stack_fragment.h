#ifndef STACK_FRAGMENT_H_INCLUDED
#define STACK_FRAGMENT_H_INCLUDED

char *getStack();

char *getSP();

void setSP(char *sp);

char *getStackAfter();

void setStackAfter(char *stackAfter);

void pusha(void *a);

void *popa();

void pushc(char c);

char popc();

void pushd(double d);

double popd();

void pushi(long i);

long popi();

#endif