#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#include <stdlib.h>
#include "lexical_analyzer.h"

#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory");
#define SAFESTRALLOC(var, len)                                    \
    if ((var = (char *)malloc((len + 1) * sizeof(char))) == NULL) \
        err("not enough memory");
#define SAFEREALLOC(var, Type, len)                                 \
    if ((var = (Type *)realloc(var, (len) * sizeof(Type))) == NULL) \
        err("not enough memory");

#define STACK_SIZE (32 * 1024)
#define GLOBAL_SIZE (32 * 1024)

#endif