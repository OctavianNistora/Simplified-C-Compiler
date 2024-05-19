#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

Instr *addInstr(int opcode);

Instr *addInstrAfter(Instr *after, int opcode);

Instr *addInstrA(int opcode, void *addr);

Instr *addInstrI(int opcode, long int val);

Instr *addInstrII(int opcode, long int val1, long int val2);

void deleteInstructionsAfter(Instr *start);

void *allocGlobal(int size);

Instr *getLastInstruction();

int typeBaseSize(Type *type);

int typeFullSize(Type *type);

int typeArgSize(Type *type);

void execute();

#endif