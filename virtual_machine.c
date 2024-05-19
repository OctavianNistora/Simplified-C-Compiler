#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "types.h"
#include "stack_fragment.h"
#include "semantic_analyzer_fragment.h"
#include "virtual_machine.h"

Instr *instructions, *lastInstruction; // double linked list
char globals[GLOBAL_SIZE];
int nGlobals;

Instr *createInstr(int opcode)
{
    Instr *i;
    SAFEALLOC(i, Instr)
    i->opcode = opcode;
    return i;
}

void insertInstrAfter(Instr *after, Instr *i)
{
    i->next = after->next;
    i->last = after;
    after->next = i;
    if (i->next == NULL)
    {
        lastInstruction = i;
    }
}

Instr *addInstr(int opcode)
{
    Instr *i = createInstr(opcode);
    i->next = NULL;
    i->last = lastInstruction;
    if (lastInstruction)
    {
        lastInstruction->next = i;
    }
    else
    {
        instructions = i;
    }
    lastInstruction = i;
    return i;
}

Instr *addInstrAfter(Instr *after, int opcode)
{
    Instr *i = createInstr(opcode);
    insertInstrAfter(after, i);
    return i;
}

Instr *addInstrA(int opcode, void *addr)
{
    Instr *i = addInstr(opcode);
    i->args[0].addr = addr;
    return i;
}

Instr *addInstrI(int opcode, long int val)
{
    Instr *i = addInstr(opcode);
    i->args[0].i = val;
    return i;
}

Instr *addInstrII(int opcode, long int val1, long int val2)
{
    Instr *i = addInstr(opcode);
    i->args[0].i = val1;
    i->args[1].i = val2;
    return i;
}

void deleteInstructionsAfter(Instr *start)
{
    Instr *i, *next;
    for (i = start->next; i; i = next)
    {
        next = i->next;
        free(i);
    }
    start->next = NULL;
    lastInstruction = start;
}

void *allocGlobal(int size)
{
    void *p = globals + nGlobals;
    if (nGlobals + size > GLOBAL_SIZE)
    {
        err("insufficient globals space");
    }
    nGlobals += size;
    return p;
}

/*
v = 3;
do
{
    put_i(v);
    v = v - 1;
} while (v);
*/
void mvTest()
{
    Instr *L1;
    int *v = allocGlobal(sizeof(long int));
    addInstrA(O_PUSHCT_A, v);
    addInstrI(O_PUSHCT_I, 3);
    addInstrI(O_STORE, sizeof(long int));
    L1 = addInstrA(O_PUSHCT_A, v);
    addInstrI(O_LOAD, sizeof(long int));
    addInstrA(O_CALLEXT, requireSymbol(getSymbolsTable(), "put_i")->addr);
    addInstrA(O_PUSHCT_A, v);
    addInstrA(O_PUSHCT_A, v);
    addInstrI(O_LOAD, sizeof(long int));
    addInstrI(O_PUSHCT_I, 1);
    addInstr(O_SUB_I);
    addInstrI(O_STORE, sizeof(long int));
    addInstrA(O_PUSHCT_A, v);
    addInstrI(O_LOAD, sizeof(long int));
    addInstrA(O_JT_I, L1);
    addInstr(O_HALT);
}

void run(Instr *IP)
{
    long int iVal1, iVal2;
    double dVal1, dVal2;
    char *aVal1;
    char *FP = 0, *oldSP;
    setSP(getStack());
    setStackAfter(getStack() + STACK_SIZE);
    while (1)
    {
        printf("%p/%ld\t", IP, getSP() - getStack());
        switch (IP->opcode)
        {
        case O_ADD_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("ADD_C\t(%ld+%ld -> %ld)\n", iVal2, iVal1, iVal2 + iVal1);
            pushi(iVal2 + iVal1);
            IP = IP->next;
            break;
        case O_ADD_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("ADD_D\t(%g+%g -> %g)\n", dVal2, dVal1, dVal2 + dVal1);
            pushd(dVal2 + dVal1);
            IP = IP->next;
            break;
        case O_ADD_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("ADD_I\t(%ld+%ld -> %ld)\n", iVal2, iVal1, iVal2 + iVal1);
            pushi(iVal2 + iVal1);
            IP = IP->next;
            break;
        case O_AND_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("AND_C\t(%ld,%ld -> %d)\n", iVal2, iVal1, iVal2 && iVal1);
            pushi(iVal2 && iVal1);
            IP = IP->next;
            break;
        case O_AND_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("AND_D\t(%g,%g -> %d)\n", dVal2, dVal1, dVal2 && dVal1);
            pushi(dVal2 && dVal1);
            IP = IP->next;
            break;
        case O_AND_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("AND_I\t(%ld,%ld -> %d)\n", iVal2, iVal1, iVal2 && iVal1);
            pushi(iVal2 && iVal1);
            IP = IP->next;
            break;
        case O_CALL:
            aVal1 = IP->args[0].addr;
            printf("CALL\t%p\n", aVal1);
            pusha(IP->next);
            IP = (Instr *)aVal1;
            break;
        case O_CALLEXT:
            printf("CALLEXT\t%p\n", IP->args[0].addr);
            (*(void (*)())IP->args[0].addr)();
            IP = IP->next;
            break;
        case O_CAST_C_D:
            iVal1 = popi();
            dVal1 = (double)iVal1;
            printf("CAST_C_D\t(%ld -> %g)\n", iVal1, dVal1);
            pushd(dVal1);
            IP = IP->next;
            break;
        case O_CAST_C_I:
            iVal1 = popi();
            printf("CAST_C_I\t(%ld -> %ld)\n", iVal1, iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_CAST_D_C:
            dVal1 = popd();
            iVal1 = (char)dVal1;
            printf("CAST_D_C\t(%g -> %ld)\n", dVal1, iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_CAST_D_I:
            dVal1 = popd();
            iVal1 = (long int)dVal1;
            printf("CAST_D_I\t(%g -> %ld)\n", dVal1, iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_CAST_I_C:
            iVal1 = popi();
            printf("CAST_I_C\t(%ld -> %ld)\n", iVal1, iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_CAST_I_D:
            iVal1 = popi();
            dVal1 = (double)iVal1;
            printf("CAST_I_D\t(%ld -> %g)\n", iVal1, dVal1);
            pushd(dVal1);
            IP = IP->next;
            break;
        case O_DIV_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("DIV_C\t(%ld/%ld -> %ld)\n", iVal2, iVal1, iVal2 / iVal1);
            pushi(iVal2 / iVal1);
            IP = IP->next;
            break;
        case O_DIV_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("DIV_D\t(%g/%g -> %g)\n", dVal2, dVal1, dVal2 / dVal1);
            pushd(dVal2 / dVal1);
            IP = IP->next;
            break;
        case O_DIV_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("DIV_I\t(%ld/%ld -> %ld)\n", iVal2, iVal1, iVal2 / iVal1);
            pushi(iVal2 / iVal1);
            IP = IP->next;
            break;
        case O_DROP:
            iVal1 = IP->args[0].i;
            printf("DROP\t%ld\n", iVal1);
            if (getSP() - iVal1 < getStack())
                err("not enough stack bytes");
            setSP(getSP() - iVal1);
            IP = IP->next;
            break;
        case O_ENTER:
            iVal1 = IP->args[0].i;
            printf("ENTER\t%ld\n", iVal1);
            pusha(FP);
            FP = getSP();
            setSP(getSP() + iVal1);
            IP = IP->next;
            break;
        case O_EQ_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("EQ_C\t(%ld==%ld -> %d)\n", iVal2, iVal1, iVal2 == iVal1);
            pushi(iVal2 == iVal1);
            IP = IP->next;
            break;
        case O_EQ_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("EQ_D\t(%g==%g -> %d)\n", dVal2, dVal1, dVal2 == dVal1);
            pushi(dVal2 == dVal1);
            IP = IP->next;
            break;
        case O_EQ_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("EQ_I\t(%ld==%ld -> %d)\n", iVal2, iVal1, iVal2 == iVal1);
            pushi(iVal2 == iVal1);
            IP = IP->next;
            break;
        case O_GREATER_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("GREATER_C\t(%ld>%ld -> %d)\n", iVal2, iVal1, iVal2 > iVal1);
            pushi(iVal2 > iVal1);
            IP = IP->next;
            break;
        case O_GREATER_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("GREATER_D\t(%g>%g -> %d)\n", dVal2, dVal1, dVal2 > dVal1);
            pushi(dVal2 > dVal1);
            IP = IP->next;
            break;
        case O_GREATER_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("GREATER_I\t(%ld>%ld -> %d)\n", iVal2, iVal1, iVal2 > iVal1);
            pushi(iVal2 > iVal1);
            IP = IP->next;
            break;
        case O_GREATEREQ_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("GREATEREQ_C\t(%ld>=%ld -> %d)\n", iVal2, iVal1, iVal2 >= iVal1);
            pushi(iVal2 >= iVal1);
            IP = IP->next;
            break;
        case O_GREATEREQ_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("GREATEREQ_D\t(%g>=%g -> %d)\n", dVal2, dVal1, dVal2 >= dVal1);
            pushi(dVal2 >= dVal1);
            IP = IP->next;
            break;
        case O_GREATEREQ_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("GREATEREQ_I\t(%ld>=%ld -> %d)\n", iVal2, iVal1, iVal2 >= iVal1);
            pushi(iVal2 >= iVal1);
            IP = IP->next;
            break;
        case O_HALT:
            printf("HALT\n");
            return;
        case O_INSERT:
            iVal1 = IP->args[0].i; // iDst
            iVal2 = IP->args[1].i; // nBytes
            printf("INSERT\t%ld,%ld\n", iVal1, iVal2);
            if (getSP() + iVal2 > getStackAfter())
                err("out of stack1");
            memmove(getSP() - iVal1 + iVal2, getSP() - iVal1, iVal1); // make room
            memmove(getSP() - iVal1, getSP() + iVal2, iVal2);         // dup
            setSP(getSP() + iVal2);
            IP = IP->next;
            break;
        case O_JF_A:
            aVal1 = popa();
            printf("JF\t%p\t(%p)\n", IP->args[0].addr, aVal1);
            IP = aVal1 ? IP->next : IP->args[0].addr;
            break;
        case O_JF_C:
            iVal1 = popi();
            printf("JF\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
            IP = iVal1 ? IP->next : IP->args[0].addr;
            break;
        case O_JF_D:
            dVal1 = popd();
            printf("JF\t%p\t(%g)\n", IP->args[0].addr, dVal1);
            IP = dVal1 ? IP->next : IP->args[0].addr;
            break;
        case O_JF_I:
            iVal1 = popi();
            printf("JF\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
            IP = iVal1 ? IP->next : IP->args[0].addr;
            break;
        case O_JMP:
            printf("JMP\t%p\n", IP->args[0].addr);
            IP = IP->args[0].addr;
            break;
        case O_JT_A:
            aVal1 = popa();
            printf("JT\t%p\t(%p)\n", IP->args[0].addr, aVal1);
            IP = aVal1 ? IP->args[0].addr : IP->next;
            break;
        case O_JT_C:
            iVal1 = popi();
            printf("JT\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
            IP = iVal1 ? IP->args[0].addr : IP->next;
            break;
        case O_JT_D:
            dVal1 = popd();
            printf("JT\t%p\t(%g)\n", IP->args[0].addr, dVal1);
            IP = dVal1 ? IP->args[0].addr : IP->next;
            break;
        case O_JT_I:
            iVal1 = popi();
            printf("JT\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
            IP = iVal1 ? IP->args[0].addr : IP->next;
            break;
        case O_LESS_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("LESS_C\t(%ld<%ld -> %d)\n", iVal2, iVal1, iVal2 < iVal1);
            pushi(iVal2 < iVal1);
            IP = IP->next;
            break;
        case O_LESS_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("LESS_D\t(%g<%g -> %d)\n", dVal2, dVal1, dVal2 < dVal1);
            pushi(dVal2 < dVal1);
            IP = IP->next;
            break;
        case O_LESS_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("LESS_I\t(%ld<%ld -> %d)\n", iVal2, iVal1, iVal2 < iVal1);
            pushi(iVal2 < iVal1);
            IP = IP->next;
            break;
        case O_LESSEQ_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("LESSEQ_C\t(%ld<=%ld -> %d)\n", iVal2, iVal1, iVal2 <= iVal1);
            pushi(iVal2 <= iVal1);
            IP = IP->next;
            break;
        case O_LESSEQ_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("LESSEQ_D\t(%g<=%g -> %d)\n", dVal2, dVal1, dVal2 <= dVal1);
            pushi(dVal2 <= dVal1);
            IP = IP->next;
            break;
        case O_LESSEQ_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("LESSEQ_I\t(%ld<=%ld -> %d)\n", iVal2, iVal1, iVal2 <= iVal1);
            pushi(iVal2 <= iVal1);
            IP = IP->next;
            break;
        case O_LOAD:
            iVal1 = IP->args[0].i;
            aVal1 = popa();
            printf("LOAD\t%ld\t(%p)\n", iVal1, aVal1);
            if (getSP() + iVal1 > getStackAfter())
                err("out of stack2");
            memcpy(getSP(), aVal1, iVal1);
            setSP(getSP() + iVal1);
            IP = IP->next;
            break;
        case O_MUL_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("MUL_C\t(%ld*%ld -> %ld)\n", iVal2, iVal1, iVal2 * iVal1);
            pushi(iVal2 * iVal1);
            IP = IP->next;
            break;
        case O_MUL_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("MUL_D\t(%g*%g -> %g)\n", dVal2, dVal1, dVal2 * dVal1);
            pushd(dVal2 * dVal1);
            IP = IP->next;
            break;
        case O_MUL_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("MUL_I\t(%ld*%ld -> %ld)\n", iVal2, iVal1, iVal2 * iVal1);
            pushi(iVal2 * iVal1);
            IP = IP->next;
            break;
        case O_NEG_C:
            iVal1 = popi();
            printf("NEG_C\t(-%ld -> %ld)\n", iVal1, -iVal1);
            pushi(-iVal1);
            IP = IP->next;
            break;
        case O_NEG_D:
            dVal1 = popd();
            printf("NEG_D\t(-%g -> %g)\n", dVal1, -dVal1);
            pushd(-dVal1);
            IP = IP->next;
            break;
        case O_NEG_I:
            iVal1 = popi();
            printf("NEG_I\t(-%ld -> %ld)\n", iVal1, -iVal1);
            pushi(-iVal1);
            IP = IP->next;
            break;
        case O_NOP:
            printf("NOP\n");
            IP = IP->next;
            break;
        case O_NOT_A:
            aVal1 = popa();
            printf("NOT_A\t(%p -> %d)\n", aVal1, !aVal1);
            pushi(!aVal1);
            IP = IP->next;
            break;
        case O_NOT_C:
            iVal1 = popi();
            printf("NOT_C\t(%ld -> %d)\n", iVal1, !iVal1);
            pushi(!iVal1);
            IP = IP->next;
            break;
        case O_NOT_I:
            iVal1 = popi();
            printf("NOT_I\t(%ld -> %d)\n", iVal1, !iVal1);
            pushi(!iVal1);
            IP = IP->next;
            break;
        case O_NOTEQ_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("NOTEQ_C\t(%ld!=%ld -> %d)\n", iVal2, iVal1, iVal2 != iVal1);
            pushi(iVal2 != iVal1);
            IP = IP->next;
            break;
        case O_NOTEQ_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("NOTEQ_D\t(%g!=%g -> %d)\n", dVal2, dVal1, dVal2 != dVal1);
            pushi(dVal2 != dVal1);
            IP = IP->next;
            break;
        case O_NOTEQ_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("NOTEQ_I\t(%ld!=%ld -> %d)\n", iVal2, iVal1, iVal2 != iVal1);
            pushi(iVal2 != iVal1);
            IP = IP->next;
            break;
        case O_OFFSET:
            iVal1 = popi();
            aVal1 = popa();
            printf("OFFSET\t(%p+%ld -> %p)\n", aVal1, iVal1, aVal1 + iVal1);
            pusha(aVal1 + iVal1);
            IP = IP->next;
            break;
        case O_OR_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("OR_C\t(%ld,%ld -> %d)\n", iVal2, iVal1, iVal2 || iVal1);
            pushi(iVal2 || iVal1);
            IP = IP->next;
            break;
        case O_OR_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("OR_D\t(%g,%g -> %d)\n", dVal2, dVal1, dVal2 || dVal1);
            pushi(dVal2 || dVal1);
            IP = IP->next;
            break;
        case O_OR_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("OR_I\t(%ld,%ld -> %d)\n", iVal2, iVal1, iVal2 || iVal1);
            pushi(iVal2 || iVal1);
            IP = IP->next;
            break;
        case O_PUSHFPADDR:
            iVal1 = IP->args[0].i;
            printf("PUSHFPADDR\t%ld\t(%p)\n", iVal1, FP + iVal1);
            pusha(FP + iVal1);
            IP = IP->next;
            break;
        case O_PUSHCT_A:
            aVal1 = IP->args[0].addr;
            printf("PUSHCT_A\t%p\n", aVal1);
            pusha(aVal1);
            IP = IP->next;
            break;
        case O_PUSHCT_C:
            iVal1 = IP->args[0].i;
            printf("PUSHCT_C\t'%c'\n", (char)iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_PUSHCT_D:
            dVal1 = IP->args[0].d;
            printf("PUSHCT_D\t%g\n", dVal1);
            pushd(dVal1);
            IP = IP->next;
            break;
        case O_PUSHCT_I:
            iVal1 = IP->args[0].i;
            printf("PUSHCT_I\t%ld\n", iVal1);
            pushi(iVal1);
            IP = IP->next;
            break;
        case O_RET:
            iVal1 = IP->args[0].i; // sizeArgs
            iVal2 = IP->args[1].i; // sizeof(retType)
            printf("RET\t%ld,%ld\n", iVal1, iVal2);
            oldSP = getSP();
            setSP(FP);
            FP = popa();
            IP = popa();
            if (getSP() - iVal1 < getStack())
                err("not enough stack bytes");
            setSP(getSP() - iVal1);
            memmove(getSP(), oldSP - iVal2, iVal2);
            setSP(getSP() + iVal2);
            break;
        case O_STORE:
            iVal1 = IP->args[0].i;
            if (getSP() - (sizeof(void *) + iVal1) < getStack())
                err("not enough stack bytes for SET");
            aVal1 = *(void **)(getSP() - ((sizeof(void *) + iVal1)));
            printf("STORE\t%ld\t(%p)\n", iVal1, aVal1);
            memcpy(aVal1, getSP() - iVal1, iVal1);
            setSP(getSP() - sizeof(void *) + iVal1);
            IP = IP->next;
            break;
        case O_SUB_C:
            iVal1 = popi();
            iVal2 = popi();
            printf("SUB_C\t(%ld-%ld -> %ld)\n", iVal2, iVal1, iVal2 - iVal1);
            pushi(iVal2 - iVal1);
            IP = IP->next;
            break;
        case O_SUB_D:
            dVal1 = popd();
            dVal2 = popd();
            printf("SUB_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 - dVal1);
            pushd(dVal2 - dVal1);
            IP = IP->next;
            break;
        case O_SUB_I:
            iVal1 = popi();
            iVal2 = popi();
            printf("SUB_I\t(%ld-%ld -> %ld)\n", iVal2, iVal1, iVal2 - iVal1);
            pushi(iVal2 - iVal1);
            IP = IP->next;
            break;
        default:
            err("invalid opcode: %d", IP->opcode);
        }
    }
}

Instr *getLastInstruction()
{
    return lastInstruction;
}

int typeBaseSize(Type *type)
{
    int size = 0;
    Symbol **is;
    switch (type->typeBase)
    {
    case TB_INT:
        size = sizeof(long int);
        break;
    case TB_DOUBLE:
        size = sizeof(double);
        break;
    case TB_CHAR:
        size = sizeof(char);
        break;
    case TB_STRUCT:
        for (is = type->s->members.begin; is != type->s->members.end; is++)
        {
            size += typeFullSize(&(*is)->type);
        }
        break;
    case TB_VOID:
        size = 0;
        break;
    default:
        err("invalid typeBase: %d", type->typeBase);
    }
    return size;
}

int typeFullSize(Type *type)
{
    return typeBaseSize(type) * (type->nElements > 0 ? type->nElements : 1);
}

int typeArgSize(Type *type)
{
    if (type->nElements >= 0)
        return sizeof(void *);
    return typeBaseSize(type);
}

void execute()
{
    run(instructions);
}

/*int main()
{
    addExtFuncs();
    mvTest();
    execute();
    return 0;
}*/