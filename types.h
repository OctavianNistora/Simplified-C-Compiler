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
    union
    {
        void *addr; // vm: the memory address for global symbols
        int offset; // vm: the stack offset for local symbols
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

enum
{
    O_ADD_C,
    O_ADD_D,
    O_ADD_I,
    O_AND_A,
    O_AND_C,
    O_AND_D,
    O_AND_I,
    O_CALL,
    O_CALLEXT,
    O_CAST_C_D,
    O_CAST_C_I,
    O_CAST_D_C,
    O_CAST_D_I,
    O_CAST_I_C,
    O_CAST_I_D,
    O_DIV_C,
    O_DIV_D,
    O_DIV_I,
    O_DROP,
    O_ENTER,
    O_EQ_A,
    O_EQ_C,
    O_EQ_D,
    O_EQ_I,
    O_GREATER_C,
    O_GREATER_D,
    O_GREATER_I,
    O_GREATEREQ_C,
    O_GREATEREQ_D,
    O_GREATEREQ_I,
    O_HALT,
    O_INSERT,
    O_JF_A,
    O_JF_C,
    O_JF_D,
    O_JF_I,
    O_JMP,
    O_JT_A,
    O_JT_C,
    O_JT_D,
    O_JT_I,
    O_LESS_C,
    O_LESS_D,
    O_LESS_I,
    O_LESSEQ_C,
    O_LESSEQ_D,
    O_LESSEQ_I,
    O_LOAD,
    O_MUL_C,
    O_MUL_D,
    O_MUL_I,
    O_NEG_C,
    O_NEG_D,
    O_NEG_I,
    O_NOP,
    O_NOT_A,
    O_NOT_C,
    O_NOT_D,
    O_NOT_I,
    O_NOTEQ_A,
    O_NOTEQ_C,
    O_NOTEQ_D,
    O_NOTEQ_I,
    O_OFFSET,
    O_OR_A,
    O_OR_C,
    O_OR_D,
    O_OR_I,
    O_PUSHFPADDR,
    O_PUSHCT_A,
    O_PUSHCT_C,
    O_PUSHCT_D,
    O_PUSHCT_I,
    O_RET,
    O_STORE,
    O_SUB_C,
    O_SUB_D,
    O_SUB_I
}; // all opcodes; each one starts with O_

typedef struct _Instr
{
    int opcode; // O_*
    union
    {
        long int i; // int, char
        double d;
        void *addr;
    } args[2];
    struct _Instr *last, *next; // links to last, next instructions
} Instr;

#endif