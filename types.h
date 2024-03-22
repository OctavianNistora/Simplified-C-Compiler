#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

enum
{
    ID,        // 0
    BREAK,     // 1
    CHAR,      // 2
    DOUBLE,    // 3
    ELSE,      // 4
    FOR,       // 5
    IF,        // 6
    INT,       // 7
    RETURN,    // 8
    STRUCT,    // 9
    VOID,      // 10
    WHILE,     // 11
    CT_INT,    // 12
    CT_REAL,   // 13
    CT_CHAR,   // 14
    CT_STRING, // 15
    COMMA,     // 16
    SEMICOLON, // 17
    LPAR,      // 18
    RPAR,      // 19
    LBRACKET,  // 20
    RBRACKET,  // 21
    LACC,      // 22
    RACC,      // 23
    ADD,       // 24
    SUB,       // 25
    MUL,       // 26
    DIV,       // 27
    DOT,       // 28
    AND,       // 29
    OR,        // 30
    NOT,       // 31
    ASSIGN,    // 32
    EQUAL,     // 33
    NOTEQ,     // 34
    LESS,      // 35
    LESSEQ,    // 36
    GREATER,   // 37
    GREATEREQ, // 38
    END        // 39
};

typedef struct _Token Token;

#endif