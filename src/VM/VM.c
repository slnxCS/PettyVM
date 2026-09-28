#include "VM.h"
#include "../Collections/stack.h"
#include "../Value/PettyValue.h"
#include "../types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define current_code(vm) (vm->OpCodes[vm->current_instruction])
#define vm_advance(vm) (vm->current_instruction++)
#define vm_advance_n(vm, len) ( vm->current_instruction+= len; )
#define vm_pop(vm) (stack_pop(&vm->stack))
#define vm_push(vm, obj) stack_push(&(vm->stack), obj)
#define _processOpCodeMath(vm, type, typeAs, opCode, operator, returnTypeAs, returnTypeKind, typeCast) \
case opCode : \
{\
    PettyValue pt_val;\
    type right = vm_pop(vm).as.typeAs;\
    type left = vm_pop(vm).as.typeAs;\
    pt_val.kind = returnTypeKind;\
    pt_val.as.returnTypeAs = (typeCast)left operator (typeCast)right;\
    stack_push(&vm->stack, pt_val);\
    break;\
}

#define _processOpCodeCast(vm, opCode, fromAsType, toAsType, castKind, castType) \
case opCode : \
{\
    PettyValue val = vm_pop(vm);\
    val.as.toAsType = (castType)val.as.fromAsType;\
    val.kind = castKind;\
    vm_push(vm, val);\
    break;\
}

typedef void(*PettySysFunc)(VM* vm, int32_t arity);

void pt_vm_sysprint_int32(VM* vm, int32_t arity) {
    //for (int i = 0; i < arity; i++) 
    {
        int32_t num = stack_pop(&(vm->stack)).as.as_int;
        printf("%d\n", num);
    }
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

void pt_vm_sysprint_bool(VM* vm, int32_t arity) {
    PettyValue value = stack_pop(&(vm->stack));
    printf("%s\n", value.as.as_bool ? "true" : "false");
}

PettySysFunc vm_sys_funcs[] = {
    pt_vm_sysprint_int32,
    pt_vm_sysprint_float32,
    pt_vm_sysprint_bool,
    pt_vm_sysread_int32,
};

void VM_addConstant(VM* vm, uint32_t constant_index) {
    unsigned char type = current_code(vm);
    vm_advance(vm);
    switch (type) {
        case CONSTANT_INT :
        {
            PettyValue num = VM_read_Int(vm);
            vm->Constants[constant_index] = num;
            break;
        }

        case CONSTANT_FLOAT : 
        {
            PettyValue num = VM_read_float(vm);
            vm->Constants[constant_index] = num;
            break;
        }

        case CONSTANT_BOOL : 
        {
            PettyValue val = VM_read_bool(vm);
            vm->Constants[constant_index] = val;
            break;
        }
    }
}

void VM_initFuncs(VM* vm, int32_t count) {
    for (int i = 0; i < count; i++) {
        vm->Functions[i] = vm->current_instruction;
        while (current_code(vm) != HALT)
            vm_advance(vm);
        vm_advance(vm);
    }
}

int VM_initHeap(VM* vm) {
    vm->heap_size = HEAP_MEMORY_OBJ_COUNT;
    vm->heap = malloc(HEAP_GET_ALLOC_COUNT_BYTES);
    for (uint64_t i = 0; i < vm->heap_size; i++) {
        vm->heap[i] = NULL;
    }
    return 0;
}

int VM_initClasses(VM* vm, uint32_t lenght) {
    for (int i = 0; i < lenght; i++) {
        PettyClass* _class = &(vm->classes[i]);
        _class->NameLength = VM_read_raw_Int(vm) + 1;
        _class->Name = malloc(_class->NameLength);
        if (!_class->Name) {
            fprintf(stderr, "Malloc error : failed to allocate %d bytes\n", _class->NameLength);
            exit(1);
        }
        VM_read_raw_string(vm, _class->Name, _class->NameLength - 1);
        _class->Name[_class->NameLength - 1] = '\0';
        _class->ID = VM_read_raw_Int(vm);
        _class->DerivedID = VM_read_raw_Int(vm);
    }

    return 0;
}

int VM_init(VM* vm, byte* input, const char* file_name)
{
    if (!input)
        return 1;
    vm->OpCodes = input;
    vm->current_instruction = 0;
    vm->frame_pointer = 0;
    char magic[7];
    strncpy(magic, (char*)input, 6);
    magic[6] = '\0';

    if (memcmp(magic, "[PTVM]", 6) != 0) 
    {
        fprintf(stderr, "File '%s' is not PettyLang bytecode. Terminating VM\n", file_name);
        exit(1);
    }
    for (int i = 0; i < 6; i++)
        vm_advance(vm);
    float32_t file_byteCodeVersion = VM_read_raw_float(vm);
    if (file_byteCodeVersion != BYTECODE_VER) {
        fprintf(stderr, "The bytecode version for this file is outdated or newer. Current VM bytecode version : %g. File bytecode version : %g. \n" 
            "Please recompile the file to match the current bytecode version of the vm, or update the vm for your file version\n", BYTECODE_VER, file_byteCodeVersion);
        exit(2);
    }

    VM_initHeap(vm);

    vm->ConstantsCount = VM_read_raw_Int(vm);
    vm->Constants = malloc(sizeof(PettyValue) * vm->ConstantsCount);
    for (uint32_t i = 0; i < vm->ConstantsCount; i++) 
        VM_addConstant(vm, i);
    int32_t globals_c = VM_read_raw_Int(vm);
    vm->Globals = malloc(sizeof(PettyValue) * globals_c);
    vm->stack = stack_init();
    VM_Start(vm);
    vm_advance(vm);
    uint32_t funcs_c = VM_read_raw_Int(vm);
    vm->Functions = malloc(sizeof(int32_t) * funcs_c);
    VM_initFuncs(vm, funcs_c);
    uint32_t classes_c = VM_read_raw_Int(vm);
    vm->classes = malloc(sizeof(PettyClass) * classes_c);
    VM_initClasses(vm, classes_c);
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

bool VM_read_raw_bool(VM* vm) {
    bool b = current_code(vm) != 0;
    vm_advance(vm);
    return b;
}

PettyValue VM_read_bool(VM* vm) {
    PettyValue val;
    val.kind = PT_BOOL;
    val.as.as_bool = VM_read_raw_bool(vm);
    return val;
}

float32_t VM_read_raw_float(VM* vm) {
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (uint8_t)current_code(vm);
        vm_advance(vm);
    }

    float32_t num;

    memcpy(&num, bytes, sizeof(float32_t));
    return num;
}

PettyValue VM_read_float(VM* vm) {
    float32_t num = VM_read_raw_float(vm);
    PettyValue val;
    val.kind = PT_FLOAT32;
    val.as.as_float = num;
    return val;
}

int32_t VM_read_raw_Int(VM* vm) {
    int32_t v = 0;

    for (int i = 0; i < 4; i++) 
    {
        v |= ((uint8_t)current_code(vm)) << (8 * i);
        vm_advance(vm);
    }

    return v;
}

void VM_read_raw_string(VM* vm, char* buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++, vm_advance(vm)) {
        buffer[i] = (char)current_code(vm);
    }
}

PettyValue VM_read_Int(VM* vm) {
    int32_t v = 0;
    PettyValue val;

    for (int i = 0; i < 4; i++) 
    {
        v |= ((uint8_t)current_code(vm)) << (8 * i);
        vm_advance(vm);
    }

    val.kind = PT_INT32;
    val.as.as_int = v;
    return val;
}

void VM_Free_Heap(VM* vm) {
    for (uint64_t i = 0; i < vm->heap_size; i++) {
        free(vm->heap[i]);
    }

    free(vm->heap);
}

void VM_Free_ClassTable(VM* vm) {

}

int VM_Free(VM* vm) {
    free(vm->Constants);
    free(vm->Globals);
    free(vm->OpCodes);
    VM_Free_Heap(vm);
    VM_Free_ClassTable(vm);

    return 0;
}

int VM_Start(VM* vm) 
{
    while (current_code(vm) != HALT) {
        VM_OpCode current = current_code(vm);
        vm_advance(vm);
        switch (current) {
            case RET : {
                Frame current_frame = vm->call_stack[--vm->frame_pointer];
                free(current_frame.locals);
                vm->current_instruction = current_frame.return_ip;
                break;
            }

            case CALL : {
                if (vm->frame_pointer >= FRAME_STACK_MAX) 
                {
                    fprintf(stderr, "Call stack overflow error!");
                    exit(5);
                }
                int32_t func_index = VM_read_raw_Int(vm);
                int32_t func_arity = VM_read_raw_Int(vm);
                Frame frame;
                frame.return_ip = vm->current_instruction;
                frame.stack_ptr_index = vm->stack.top_index;
                frame.locals = NULL;
                frame.arity = func_arity;
                vm->call_stack[vm->frame_pointer++] = frame;
                vm->current_instruction = vm->Functions[func_index];
                break;
            }

            case RESERVE_LOCAL : 
            {
                int32_t count = VM_read_raw_Int(vm);
                vm->call_stack[vm->frame_pointer - 1].locals = malloc(sizeof(PettyValue) * count);
                for (uint32_t i = 0; i < vm->call_stack[vm->frame_pointer - 1].arity; i++) {
                    vm->call_stack[vm->frame_pointer - 1].locals[i] = vm_pop(vm);
                }
                break;
            }

            case SYS_CALL :
            {
                int32_t sys_func_index = VM_read_raw_Int(vm);
                int32_t arity = VM_read_raw_Int(vm);
                vm_sys_funcs[sys_func_index](vm, arity);
                break;
            }

            case PUSH_CONSTANT :  
            {
                int32_t index = VM_read_raw_Int(vm);
                vm_push(vm, vm->Constants[index]);
                break;
            }

            case STORE_GLOBAL : 
            {
                int32_t index = VM_read_raw_Int(vm);
                PettyValue obj = stack_pop(&(vm->stack));
                vm->Globals[index] = obj;
                break;
            }

            case STORE_LOCAL : 
            {  
                int32_t index = VM_read_raw_Int(vm);
                PettyValue obj = vm_pop(vm);
                //vm.stack.ptr[vm.call_stack[vm.frame_pointer - 1].stack_ptr_index + index] = obj;
                vm->call_stack[vm->frame_pointer - 1].locals[index] = obj;
                break;
            }

            case LOAD_LOCAL : 
            {  
                int32_t index = VM_read_raw_Int(vm);
                vm_push(vm, vm->call_stack[vm->frame_pointer - 1].locals[index]);
                break;
            }

            case LOAD_GLOBAL : 
            {
                int32_t index = VM_read_raw_Int(vm);
                vm_push(vm, vm->Globals[index]);
                break;
            }

            case JMP : 
            {
                vm->current_instruction = VM_read_raw_Int(vm);
                break;
            }

            case JMP_IF_FALSE : 
            {
                PettyValue val = vm_pop(vm);
                int32_t pos = VM_read_raw_Int(vm);
                if (!val.as.as_bool) 
                {
                    vm->current_instruction = pos;
                }
                break;
            }

            case JMP_IF_TRUE : 
            {
                PettyValue val = vm_pop(vm);
                int32_t pos = VM_read_raw_Int(vm);
                if (val.as.as_bool) 
                {
                    vm->current_instruction = pos;
                }
                break;
            }

            _processOpCodeCast(vm, CAST_FROM_FLOAT32_TO_INT32, as_float, as_int, PT_INT32, int32_t)
            _processOpCodeCast(vm, CAST_FROM_INT32_TO_FLOAT32, as_int, as_float, PT_FLOAT32, float32_t)

            _processOpCodeMath(vm, int32_t, as_int, INT_EQ, ==, as_bool, PT_BOOL, int32_t)
            _processOpCodeMath(vm, int32_t, as_int, ADD_INT, +, as_int, PT_INT32, int32_t)
            _processOpCodeMath(vm, int32_t, as_int, SUB_INT, -, as_int, PT_INT32, int32_t)
            _processOpCodeMath(vm, int32_t, as_int, MUL_INT, *, as_int, PT_INT32, int32_t)
            _processOpCodeMath(vm, int32_t, as_int, DIV_INT, /, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(vm, float32_t, as_float, ADD_FLOAT, +, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(vm, float32_t, as_float, SUB_FLOAT, -, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(vm, float32_t, as_float, MUL_FLOAT, *, as_float, PT_FLOAT32, float32_t)
            _processOpCodeMath(vm, float32_t, as_float, DIV_FLOAT, /, as_float, PT_FLOAT32, float32_t)

            case HALT : goto vm_end;

            default: 
            {
                fprintf(stderr, "VM operations handler error : Unknown operation code (%d)\n", current);
                exit(5);
            }
        }
    }

    vm_end :
    
    return 0;
}
