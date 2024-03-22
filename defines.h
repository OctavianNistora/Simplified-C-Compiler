#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory");
#define SAFESTRALLOC(var, len)                                    \
    if ((var = (char *)malloc((len + 1) * sizeof(char))) == NULL) \
        err("not enough memory");

#endif