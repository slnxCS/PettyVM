#ifndef VM_H
#define VM_H

#include "../Value/Objects/PettyClass.h"
#include "../Value/Objects/PettyObject.h"
#include "../Value/PettyValue.h"
#include "../types.h"
#include "../Collections/stack.h"

#define BYTECODE_VER (1.35f)

#define FRAME_STACK_MAX (100)
#define HEAP_MEMORY_OBJ_COUNT (128)
#define HEAP_GET_ALLOC_COUNT_BYTES (HEAP_MEMORY_OBJ_COUNT * sizeof(PettyObject))

typedef enum VM_OpCode : byte {
    PUSH_CONSTANT = 1,
    STORE_LOCAL = 2,
    STORE_GLOBAL = 3,
    SYS_CALL = 4,
    LOAD_LOCAL = 5,
    LOAD_GLOBAL = 6,
    RET = 7,
    CALL = 8,
    RESERVE_LOCAL = 9,

    ADD_INT = 10,
    SUB_INT = 11,
    DIV_INT = 12,
    MUL_INT = 13,
    CAST_FROM_FLOAT32_TO_INT32 = 14,
    HALT = 15,
    CAST_FROM_INT32_TO_FLOAT32 = 16,
    ADD_FLOAT = 17,
    SUB_FLOAT = 18,
    MUL_FLOAT = 19,
    DIV_FLOAT = 20,
    JMP_IF_FALSE = 26,
    JMP_IF_TRUE = 27,
    JMP = 28,
    INT_EQ = 29,
} VM_OpCode;

typedef enum VM_ConstantType {
    CONSTANT_INT = 0,
    CONSTANT_FLOAT = 1,
    CONSTANT_BOOL = 2,
} VM_ConstantType;

typedef struct Frame Frame;

struct Frame {
    uint32_t return_ip;
    uint32_t stack_ptr_index;
    uint32_t arity;
};

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
    int* Functions;
    Frame call_stack[FRAME_STACK_MAX];
    int32_t frame_pointer;
    uint64_t heap_size;
    PettyObject** heap;
    uint32_t classes_count;
    PettyClass* classes;
};

extern int32_t VM_read_raw_Int(VM* vm);
extern PettyValue VM_read_Int(VM* vm);
extern float32_t VM_read_raw_float(VM* vm);
extern PettyValue VM_read_bool(VM* vm);
extern bool VM_read_raw_bool(VM* vm);
extern PettyValue VM_read_float(VM* vm);
extern void VM_read_raw_string(VM* vm, char* buffer, uint32_t length);
extern int VM_init(VM* vm, byte* input, const char* file_name);
extern int VM_Start(VM* vm);
extern int VM_Free(VM* vm);

#endif // VM_H