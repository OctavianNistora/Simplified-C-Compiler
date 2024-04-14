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

typedef struct _Token
{
    int code;
    union
    {
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _Token *next;
} Token;

enum // Types enum
{
    TB_INT,
    TB_DOUBLE,
    TB_CHAR,
    TB_STRUCT,
    TB_VOID
};
typedef struct _Symbol Symbol;
typedef struct
{
    int typeBase;  // TB_*
    Symbol *s;     // struct definition for TB_STRUCT
    int nElements; // >0 array of given size, 0=array without size, <0 non array
} Type;
enum // Class enum
{
    CLS_VAR,
    CLS_FUNC,
    CLS_EXTFUNC,
    CLS_STRUCT
};
enum // Memory enum
{
    MEM_GLOBAL,
    MEM_ARG,
    MEM_LOCAL
};
typedef struct
{
    Symbol **begin; // the beginning of the symbols, or NULL
    Symbol **end;   // the position after the last symbol
    Symbol **after; // the position after the allocated space
} Symbols;

typedef struct _Symbol
{
    const char *name; // a reference to the name stored in a token
    int cls;          // CLS_*
    int mem;          // MEM_*
    Type type;
    int depth; // 0-global, 1-in function, 2... - nested blocks in function
    union
    {
        Symbols args;    // used only of functions
        Symbols members; // used only for structs
    };
} Symbol;

typedef union
{
    long int i;      // int, char
    double d;        // double
    const char *str; // char[]
} CtVal;

typedef struct
{
    Type type;   // type of the result
    int isLVal;  // if it is a LVal
    int isCtVal; // if it is a constant value (int, real, char, char[])
    CtVal ctVal; // the constat value
} RetVal;

#endif