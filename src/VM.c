#include "VM.h"
#include "Collections/stack.h"
#include "PettyValue.h"
#include "types.h"
#include <assert.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define current_code (vm.OpCodes[vm.current_instruction])
#define vm_advance (vm.current_instruction++)
#define vm_pop (stack_pop(&vm.stack))
#define vm_push(obj) stack_push(&vm.stack, obj)
#define _processOpCodeMath(type, typeAs, opCode, operator, returnTypeAs, returnTypeKind, typeCast) \
case opCode : \
{\
    PettyValue pt_val;\
    type right = vm_pop.as.typeAs;\
    type left = vm_pop.as.typeAs;\
    pt_val.kind = returnTypeKind;\
    pt_val.as.returnTypeAs = (typeCast)left operator (typeCast)right;\
    stack_push(&vm.stack, pt_val);\
    break;\
}

typedef void(*PettySysFunc)(VM* vm, int32_t arity);

void pt_vm_sysprint_int32(VM* vm, int32_t arity) {
    int32_t num = stack_pop(&(vm->stack)).as.as_int;
    printf("%d\n", num);
}

void pt_vm_sysprint_float32(VM* vm, int32_t arity) {
    float32_t num = stack_pop(&(vm->stack)).as.as_float;
    printf("%g\n", num);
}

void pt_vm_sysread_int32(VM* vm, int32_t arity) {
    PettyValue val;
    val.kind = PT_INT32;
    scanf("%d", &(val.as.as_int));
    stack_push(&vm->stack, val);
}

PettySysFunc vm_sys_funcs[] = {
    pt_vm_sysprint_int32,
    pt_vm_sysprint_float32,
    NULL,
    pt_vm_sysread_int32,
};

VM vm;

void VM_addConstant(uint32_t constant_index) {
    unsigned char type = current_code;
    vm_advance;
    switch (type) {
        case CONSTANT_INT :
        {
            PettyValue num = VM_read_Int();
            vm.Constants[constant_index] = num;
            break;
        }

        case CONSTANT_FLOAT : 
        {
            PettyValue num = VM_read_float();
            vm.Constants[constant_index] = num;
            break;
        }
    }
}

PettyValue Int32Class, Float32Class, VoidClass, ObjectClass, FunctionClass;

void VM_initBuiltins() 
{
    vm.Globals[0] = Int32Class;
    vm.Globals[1] = Float32Class;
    vm.Globals[2] = VoidClass;
    vm.Globals[3] = ObjectClass;
    vm.Globals[4] = FunctionClass;
}

void VM_initFuncs(int32_t count) {
    for (int i = 0; i < count; i++) {
        vm.Functions[i] = vm.current_instruction;
        while (current_code != HALT)
            vm_advance;
        vm_advance;
    }
}

int VM_init(byte* input, const char* file_name)
{
    if (!input)
        return 1;
    vm.OpCodes = input;
    vm.current_instruction = 0;
    vm.frame_pointer = 0;
    char magic[7];
    strncpy(magic, (char*)input, 6);
    magic[6] = '\0';

    if (memcmp(magic, "[PTVM]", 6) != 0) 
    {
        fprintf(stderr, "File '%s' is not PettyLang bytecode. Terminating VM\n", file_name);
        exit(1);
    }
    for (int i = 0; i < 6; i++)
        vm_advance;
    float32_t file_byteCodeVersion = VM_read_raw_float();
    if (file_byteCodeVersion != BYTECODE_VER) {
        fprintf(stderr, "The bytecode version for this file is outdated or newer. Current VM bytecode version : %f. File bytecode version : %f. \n" 
            "Please recompile the file to match the current bytecode version of the vm, or update the vm for your file version\n", BYTECODE_VER, file_byteCodeVersion);
        exit(2);
    }
    vm.ConstantsCount = VM_read_raw_Int();
    vm.Constants = malloc(sizeof(PettyValue) * vm.ConstantsCount);
    for (uint32_t i = 0; i < vm.ConstantsCount; i++) 
        VM_addConstant(i);
    int32_t globals_c = VM_read_raw_Int();
    vm.Globals = malloc(sizeof(PettyValue) * globals_c);
    vm.stack = stack_init();
    VM_initBuiltins();
    VM_Start();
    vm_advance;
    int32_t funcs_c = VM_read_raw_Int();
    vm.Functions = malloc(sizeof(int32_t) * funcs_c);
    VM_initFuncs(funcs_c);
    return 0;
}

//uint64_t VM_read_ID() {
//    uint64_t v = 0;
//
//    for (int i = 0; i < 8; i++) 
//    {
//        v |= ((uint8_t)current_code) << (8 * i);
//        vm_advance;
//    }
//
//    return v;
//}

float32_t VM_read_raw_float() {
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (uint8_t)current_code;
        vm_advance;
    }

    float32_t num;

    memcpy(&num, bytes, sizeof(float32_t));
    return num;
}

PettyValue VM_read_float() {
    float32_t num = VM_read_raw_float();
    PettyValue val;
    val.kind = PT_FLOAT32;
    val.as.as_float = num;
    return val;
}

int32_t VM_read_raw_Int() {
    int32_t v = 0;

    for (int i = 0; i < 4; i++) 
    {
        v |= ((uint8_t)current_code) << (8 * i);
        vm_advance;
    }

    return v;
}

PettyValue VM_read_Int() {
    int32_t v = 0;
    PettyValue val;

    for (int i = 0; i < 4; i++) 
    {
        v |= ((uint8_t)current_code) << (8 * i);
        vm_advance;
    }

    val.kind = PT_INT32;
    val.as.as_int = v;
    return val;
}

int VM_free() {
    free(vm.Constants);
    free(vm.Globals);
    free(vm.OpCodes);

    return 0;
}

int VM_Start() 
{
    while (current_code != HALT) {
        VM_OpCode current = current_code;
        vm_advance;
        switch (current) {
            case RET : {
                vm.current_instruction = vm.call_stack[--vm.frame_pointer].return_ip;
                break;
            }

            case CALL : {
                if (vm.frame_pointer >= FRAME_STACK_MAX) 
                {
                    fprintf(stderr, "Stack overflow error!");
                    exit(5);
                }
                int32_t func_index = VM_read_raw_Int();
                int32_t func_arity = VM_read_raw_Int();
                Frame frame;
                frame.return_ip = vm.current_instruction;
                frame.stack_ptr_index = vm.stack.top_index - func_arity;
                vm.call_stack[vm.frame_pointer++] = frame;
                vm.current_instruction = vm.Functions[func_index];
                break;
            }

            case RESERVE_LOCAL : 
            {
                int32_t count = VM_read_raw_Int();
                for (int i = 0; i < count; i++) {
                    PettyValue val;
                    vm_push(val);
                }
                break;
            }

            case SYS_CALL :
            {
                int32_t sys_func_index = VM_read_raw_Int();
                int32_t arity = VM_read_raw_Int();
                vm_sys_funcs[sys_func_index](&vm, arity);
                break;
            }

            case PUSH_CONSTANT :  
            {
                int32_t index = VM_read_raw_Int();
                vm_push(vm.Constants[index]);
                break;
            }

            case STORE_GLOBAL : 
            {
                int32_t index = VM_read_raw_Int();
                PettyValue obj = stack_pop(&vm.stack);
                vm.Globals[index] = obj;
                break;
            }

            case STORE_LOCAL : 
            {  
                int32_t index = VM_read_raw_Int();
                PettyValue obj = vm_pop;
                vm.stack.ptr[vm.call_stack[vm.frame_pointer - 1].stack_ptr_index + index] = obj;
                break;
            }

            case LOAD_LOCAL : 
            {  
                int32_t index = VM_read_raw_Int();
                vm_push(vm.stack.ptr[vm.call_stack[vm.frame_pointer - 1].stack_ptr_index + index]);
                break;
            }

            case LOAD_GLOBAL : 
            {
                int32_t index = VM_read_raw_Int();
                vm_push(vm.Globals[index]);
                break;
            }

            _processOpCodeMath(int32_t, as_int, ADD_INT, +, as_int, PT_INT32, int32_t)
            _processOpCodeMath(int32_t, as_int, SUB_INT, -, as_int, PT_INT32, int32_t)
            _processOpCodeMath(int32_t, as_int, MUL_INT, *, as_int, PT_INT32, int32_t)
            _processOpCodeMath(int32_t, as_int, DIV_INT, /, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(float32_t, as_float, ADD_FLOAT, +, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(float32_t, as_float, SUB_FLOAT, -, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(float32_t, as_float, MUL_FLOAT, *, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(float32_t, as_float, DIV_FLOAT, /, as_float, PT_FLOAT32, float32_t)

            case HALT : goto vm_end;
        }
    }

    vm_end :
    
    return 0;
}