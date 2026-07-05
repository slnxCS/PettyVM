#ifndef VM_H
#define VM_H

#include "PettyValue.h"
#include "types.h"
#include "Collections/stack.h"
typedef enum VM_OpCode {
    PUSH_INT = 1,
    STORE_LOCAL = 2,
    STORE_GLOBAL = 3,
    LOAD_LOCAL = 5,
    LOAD_GLOBAL = 6,

    ADD_INT = 10,
    SUB_INT = 11,
    DIV_INT = 12,
    MUL_INT = 13,
} VM_OpCode;

typedef struct VM VM;

struct VM 
{
    byte* OpCodes;
    uint64_t OpLenght;
    int current_instruction;
    stack stack;
    PettyValue* Globals;
    uint64_t GlobalsLen;
};

#endif // VM_H