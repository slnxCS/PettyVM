#ifndef VM_H
#define VM_H

#include "PettyValue.h"
#include "types.h"
#include "Collections/stack.h"

#define BYTECODE_VER 1

typedef enum VM_OpCode : byte {
    PUSH_CONSTANT = 1,
    STORE_LOCAL = 2,
    STORE_GLOBAL = 3,
    LOAD_LOCAL = 5,
    LOAD_GLOBAL = 6,

    ADD_INT = 10,
    SUB_INT = 11,
    DIV_INT = 12,
    MUL_INT = 13,

    HALT = 15,
} VM_OpCode;

typedef enum VM_ConstantType {
    CONSTANT_INT = 0,
} VM_ConstantType;

typedef struct VM VM;

struct VM 
{
    byte* OpCodes;
    int current_instruction;
    stack stack;
    PettyValue* Globals;
    //uint64_t GlobalsLen;
    PettyValue* Constants;
    uint32_t ConstantsCount;
};

extern int32_t VM_read_raw_Int();
extern PettyValue VM_read_Int();
extern int VM_init(byte* input, const char* file_name);
extern int VM_Start();

#endif // VM_H