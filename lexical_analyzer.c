#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexical_analyzer.h"

void err(const char *fmt, ...);

#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory");
#define SAFESTRALLOC(var, len)                              \
    if ((var = (char *)malloc(len * sizeof(char))) == NULL) \
        err("not enough memory");

struct _Token
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
};

int line = 1;
Token *tokens = NULL, *lastToken = NULL;
char *pCrtCh;

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken)
    {
        lastToken->next = tk;
    }
    else
    {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

char *createString(const char *pStartCh, const char *pCrtCh)
{
    char *s;
    int n = pCrtCh - pStartCh;
    SAFESTRALLOC(s, n + 1);
    for (int i = 0; i < n; i++)
    {
        s[i] = pStartCh[i];
    }
    s[n] = '\0';
    return s;
}

char getEscapeChar(char *pStartCh)
{
    char escape = *(pStartCh + 1);
    if (escape == 'a')
    {
        return '\a';
    }
    else if (escape == 'b')
    {
        return '\b';
    }
    else if (escape == 'f')
    {
        return '\f';
    }
    else if (escape == 'n')
    {
        return '\n';
    }
    else if (escape == 'r')
    {
        return '\r';
    }
    else if (escape == 't')
    {
        return '\t';
    }
    else if (escape == 'v')
    {
        return '\v';
    }
    else if (escape == '\'')
    {
        return '\'';
    }
    else if (escape == '?')
    {
        return '\?';
    }
    else if (escape == '"')
    {
        return '\"';
    }
    else if (escape == '\\')
    {
        return '\\';
    }
    else
    {
        return '\0';
    }
}

void escapeString(char *pStartCh)
{
    int n = strlen(pStartCh);
    for (int i = 0; pStartCh[i]; i++)
    {
        if (pStartCh[i] == '\\')
        {
            char escape = getEscapeChar((pStartCh + i));
            pStartCh[i] = escape;
            memmove(pStartCh + i + 1, pStartCh + i + 2, n - 1);
            n--;
        }
        n--;
    }
}

int getNextToken()
{
    int nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    while (1)
    {
        switch (ch = *pCrtCh)
        {
        case 0:
            addTk(END);
            return END;
        case '\'':
            pCrtCh++;
            if (*pCrtCh == '\\')
            {
                pCrtCh++;
                if (*pCrtCh == 'a' || *pCrtCh == 'b' || *pCrtCh == 'f' || *pCrtCh == 'n' || *pCrtCh == 'r' || *pCrtCh == 't' || *pCrtCh == 'v' || *pCrtCh == '\'' || *pCrtCh == '?' || *pCrtCh == '\"' || *pCrtCh == '\\' || *pCrtCh == '0')
                {
                    pCrtCh++;
                    if (*pCrtCh == '\'')
                    {
                        pCrtCh++;
                        tk = addTk(CT_CHAR);
                        tk->i = getEscapeChar((pCrtCh - 3));
                        return CT_CHAR;
                    }
                    else
                    {
                        tkerr(addTk(END), "invalid character1");
                    }
                }
            }
            else if (*pCrtCh == 0)
            {
                tkerr(addTk(END), "invalid character2");
            }
            else
            {
                pCrtCh++;
                if (*pCrtCh == '\'')
                {
                    pCrtCh++;
                    tk = addTk(CT_CHAR);
                    tk->i = *(pCrtCh - 2);
                    return CT_CHAR;
                }
                tkerr(addTk(END), "invalid character3");
            }
        case '"':
            pCrtCh++;
            pStartCh = pCrtCh;
            while (*pCrtCh != 0 && *pCrtCh != '"')
            {
                if (*pCrtCh == '\\')
                {
                    pCrtCh++;
                    if (*pCrtCh == 'a' || *pCrtCh == 'b' || *pCrtCh == 'f' || *pCrtCh == 'n' || *pCrtCh == 'r' || *pCrtCh == 't' || *pCrtCh == 'v' || *pCrtCh == '\'' || *pCrtCh == '?' || *pCrtCh == '\"' || *pCrtCh == '\\' || *pCrtCh == '0')
                    {
                        pCrtCh++;
                    }
                    else
                    {
                        tkerr(addTk(END), "invalid character4");
                    }
                }
                else
                {
                    pCrtCh++;
                }
            }
            if (*pCrtCh == 0)
            {
                tkerr(addTk(END), "invalid character5");
            }
            tk = addTk(CT_STRING);
            tk->text = createString(pStartCh, pCrtCh);
            escapeString(tk->text);
            pCrtCh++;
            return CT_STRING;
        case ',':
            pCrtCh++;
            addTk(COMMA);
            return COMMA;
        case ';':
            pCrtCh++;
            addTk(SEMICOLON);
            return SEMICOLON;
        case '(':
            pCrtCh++;
            addTk(LPAR);
            return LPAR;
        case ')':
            pCrtCh++;
            addTk(RPAR);
            return RPAR;
        case '[':
            pCrtCh++;
            addTk(LBRACKET);
            return LBRACKET;
        case ']':
            pCrtCh++;
            addTk(RBRACKET);
            return RBRACKET;
        case '{':
            pCrtCh++;
            addTk(LACC);
            return LACC;
        case '}':
            pCrtCh++;
            addTk(RACC);
            return RACC;
        case '+':
            pCrtCh++;
            addTk(ADD);
            return ADD;
        case '-':
            pCrtCh++;
            addTk(SUB);
            return SUB;
        case '*':
            pCrtCh++;
            addTk(MUL);
            return MUL;
        case '/':
            pCrtCh++;
            if (*pCrtCh == '/')
            {
                pCrtCh++;
                while (*pCrtCh != '\n' && *pCrtCh != '\r' && *pCrtCh != '\0')
                {
                    pCrtCh++;
                }
                break;
            }
            else if (*pCrtCh == '*')
            {
                pCrtCh++;
                while (1)
                {
                    if (*pCrtCh == '*')
                    {
                        while (*(pCrtCh + 1) == '*')
                        {
                            pCrtCh++;
                        }
                        if (*pCrtCh == 0)
                        {
                            tkerr(addTk(END), "invalid character6");
                        }
                        if (*(pCrtCh + 1) == '/')
                        {
                            break;
                        }
                    }
                    pCrtCh++;
                }
            }
            addTk(DIV);
            return DIV;
        case '.':
            pCrtCh++;
            addTk(DOT);
            return DOT;
        case '&':
            pCrtCh++;
            if (*pCrtCh == '&')
            {
                pCrtCh++;
                addTk(AND);
                return AND;
            }
            tkerr(addTk(END), "invalid character7");
        case '|':
            pCrtCh++;
            if (*pCrtCh == '|')
            {
                pCrtCh++;
                addTk(OR);
                return OR;
            }
            tkerr(addTk(END), "invalid character8");
        case '!':
            pCrtCh++;
            if (*pCrtCh == '=')
            {
                pCrtCh++;
                addTk(NOTEQ);
                return NOTEQ;
            }
            addTk(NOT);
            return NOT;
        case '=':
            pCrtCh++;
            if (*pCrtCh == '=')
            {
                pCrtCh++;
                addTk(EQUAL);
                return EQUAL;
            }
            addTk(ASSIGN);
            return ASSIGN;
        case '<':
            pCrtCh++;
            if (*pCrtCh == '=')
            {
                pCrtCh++;
                addTk(LESSEQ);
                return LESSEQ;
            }
            addTk(LESS);
            return LESS;
        case '>':
            pCrtCh++;
            if (*pCrtCh == '=')
            {
                pCrtCh++;
                addTk(GREATEREQ);
                return GREATEREQ;
            }
            addTk(GREATER);
            return GREATER;
        case ' ':
            pCrtCh++;
            break;
        case '\r':
            pCrtCh++;
            break;
        case '\t':
            pCrtCh++;
            break;
        case '\n':
            line++;
            pCrtCh++;
            break;
        default:
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
            {
                pStartCh = pCrtCh;
                pCrtCh++;
                while ((*pCrtCh >= 'a' && *pCrtCh <= 'z') || (*pCrtCh >= 'A' && *pCrtCh <= 'Z') || (*pCrtCh >= '0' && *pCrtCh <= '9') || *pCrtCh == '_')
                {
                    pCrtCh++;
                }
                nCh = pCrtCh - pStartCh;
                if (nCh == 5 && !memcmp(pStartCh, "break", 5))
                {
                    tk = addTk(BREAK);
                }
                else if (nCh == 4 && !memcmp(pStartCh, "char", 4))
                {
                    tk = addTk(CHAR);
                }
                else if (nCh == 6 && !memcmp(pStartCh, "double", 6))
                {
                    tk = addTk(DOUBLE);
                }
                else if (nCh == 4 && !memcmp(pStartCh, "else", 4))
                {
                    tk = addTk(ELSE);
                }
                else if (nCh == 3 && !memcmp(pStartCh, "for", 3))
                {
                    tk = addTk(FOR);
                }
                else if (nCh == 2 && !memcmp(pStartCh, "if", 2))
                {
                    tk = addTk(IF);
                }
                else if (nCh == 3 && !memcmp(pStartCh, "int", 3))
                {
                    tk = addTk(INT);
                }
                else if (nCh == 6 && !memcmp(pStartCh, "return", 6))
                {
                    tk = addTk(RETURN);
                }
                else if (nCh == 6 && !memcmp(pStartCh, "struct", 6))
                {
                    tk = addTk(STRUCT);
                }
                else if (nCh == 4 && !memcmp(pStartCh, "void", 4))
                {
                    tk = addTk(VOID);
                }
                else if (nCh == 5 && !memcmp(pStartCh, "while", 5))
                {
                    tk = addTk(WHILE);
                }
                else
                {
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            }
            else if (ch >= '0' && ch <= '9')
            {
                pStartCh = pCrtCh;
                pCrtCh++;
                if (*pStartCh == '0')
                {
                    if (*pCrtCh == 'x')
                    {
                        pCrtCh++;
                        if (!((*pCrtCh >= '0' && *pCrtCh <= '9') || (*pCrtCh >= 'a' && *pCrtCh <= 'f') || (*pCrtCh >= 'A' && *pCrtCh <= 'F')))
                        {
                            tkerr(addTk(END), "invalid character9");
                        }
                        pCrtCh++;
                        while ((*pCrtCh >= '0' && *pCrtCh <= '9') || (*pCrtCh >= 'a' && *pCrtCh <= 'f') || (*pCrtCh >= 'A' && *pCrtCh <= 'F'))
                        {
                            pCrtCh++;
                        }
                        tk = addTk(CT_INT);
                        tk->i = strtol(pStartCh, NULL, 16);
                        return CT_INT;
                    }
                    else if (*pCrtCh >= '0' && *pCrtCh <= '7')
                    {
                        pCrtCh++;
                        while (*pCrtCh >= '0' && *pCrtCh <= '7')
                        {
                            pCrtCh++;
                        }
                        tk = addTk(CT_INT);
                        tk->i = strtol(pStartCh, NULL, 8);
                        return CT_INT;
                    }
                    else if (*pCrtCh == '.')
                    {
                        pCrtCh++;
                        if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                        {
                            tkerr(addTk(END), "invalid character10");
                        }
                        pCrtCh++;
                        while (*pCrtCh >= '0' && *pCrtCh <= '9')
                        {
                            pCrtCh++;
                        }
                        if (*pCrtCh == 'e' || *pCrtCh == 'E')
                        {
                            pCrtCh++;
                            if (*pCrtCh == '+' || *pCrtCh == '-')
                            {
                                pCrtCh++;
                            }
                            if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                            {
                                tkerr(addTk(END), "invalid character11");
                            }
                            pCrtCh++;
                            while (*pCrtCh >= '0' && *pCrtCh <= '9')
                            {
                                pCrtCh++;
                            }
                        }
                        tk = addTk(CT_REAL);
                        tk->r = strtod(pStartCh, NULL);
                        return CT_REAL;
                    }
                    else if (*pCrtCh == 'e' || *pCrtCh == 'E')
                    {
                        pCrtCh++;
                        if (*pCrtCh == '+' || *pCrtCh == '-')
                        {
                            pCrtCh++;
                        }
                        if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                        {
                            tkerr(addTk(END), "invalid character12");
                        }
                        pCrtCh++;
                        while (*pCrtCh >= '0' && *pCrtCh <= '9')
                        {
                            pCrtCh++;
                        }
                        tk = addTk(CT_REAL);
                        tk->r = strtod(pStartCh, NULL);
                        return CT_REAL;
                    }
                    else
                    {
                        tk = addTk(CT_INT);
                        tk->i = strtol(pStartCh, NULL, 8);
                        return CT_INT;
                    }
                }
                else if (*pStartCh >= '1' && *pStartCh <= '9')
                {
                    while (*pCrtCh >= '0' && *pCrtCh <= '9')
                    {
                        pCrtCh++;
                    }
                    if (*pCrtCh == '.')
                    {
                        pCrtCh++;
                        if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                        {
                            tkerr(addTk(END), "invalid character13");
                        }
                        pCrtCh++;
                        while (*pCrtCh >= '0' && *pCrtCh <= '9')
                        {
                            pCrtCh++;
                        }
                        if (*pCrtCh == 'e' || *pCrtCh == 'E')
                        {
                            pCrtCh++;
                            if (*pCrtCh == '+' || *pCrtCh == '-')
                            {
                                pCrtCh++;
                            }
                            if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                            {
                                tkerr(addTk(END), "invalid character14");
                            }
                            pCrtCh++;
                            while (*pCrtCh >= '0' && *pCrtCh <= '9')
                            {
                                pCrtCh++;
                            }
                        }
                        tk = addTk(CT_REAL);
                        tk->r = strtod(pStartCh, NULL);
                        return CT_REAL;
                    }
                    else if (*pCrtCh == 'e' || *pCrtCh == 'E')
                    {
                        pCrtCh++;
                        if (*pCrtCh == '+' || *pCrtCh == '-')
                        {
                            pCrtCh++;
                        }
                        if (!(*pCrtCh >= '0' && *pCrtCh <= '9'))
                        {
                            tkerr(addTk(END), "invalid character15");
                        }
                        pCrtCh++;
                        while (*pCrtCh >= '0' && *pCrtCh <= '9')
                        {
                            pCrtCh++;
                        }
                        tk = addTk(CT_REAL);
                        tk->r = strtod(pStartCh, NULL);
                        return CT_REAL;
                    }
                    else
                    {
                        tk = addTk(CT_INT);
                        tk->i = strtol(pStartCh, NULL, 10);
                        return CT_INT;
                    }
                }
                else
                {
                    tkerr(addTk(END), "invalid character16");
                }
            }
            else
            {
                tkerr(addTk(END), "invalid character17");
            }
        }
    }
}

int getTokenCode(Token *tk)
{
    return tk->code;
}

char *getTokenText(Token *tk)
{
    return tk->text;
}

long int getTokenI(Token *tk)
{
    return tk->i;
}

double getTokenR(Token *tk)
{
    return tk->r;
}

Token *getTokenNext(Token *tk)
{
    return tk->next;
}

char *getPCrtCh()
{
    return pCrtCh;
}

void setPCrtCh(char *new)
{
    pCrtCh = new;
}

Token *getTokens()
{
    return tokens;
}
