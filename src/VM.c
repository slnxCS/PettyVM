#include "VM.h"
#include "Collections/stack.h"
#include "PettyValue.h"
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define current_code (vm.OpCodes[vm.current_instruction])
#define vm_advance (vm.current_instruction++)
#define vm_pop (stack_pop(&vm.stack))
#define vm_push(obj) stack_push(&vm.stack, obj)

VM vm;

void VM_addConstant(uint32_t constant_index) {
    unsigned char type = current_code;
    vm_advance;
    switch (type) {
        case CONSTANT_INT :
        {
            PettyValue num = VM_read_Int();
            vm.Constants[constant_index] = num;
            return;
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

int VM_init(byte* input, const char* file_name)
{
    if (!input)
        return 1;
    vm.OpCodes = input;
    vm.current_instruction = 0;
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
    int32_t file_byteCodeVersion = VM_read_raw_Int();
    if (file_byteCodeVersion != BYTECODE_VER) {
        fprintf(stderr, "The bytecode version for this file is outdated or newer. Current VM bytecode version : %d. File bytecode version : %d. \n" 
            "Please recompile the file to match the current bytecode version of the vm, or update the vm for your file version\n", BYTECODE_VER, file_byteCodeVersion);
        exit(2);
    }
    int32_t globals_len = VM_read_raw_Int();
    vm.Globals = malloc(sizeof(PettyValue) * globals_len);
    vm.ConstantsCount = VM_read_raw_Int();
    vm.Constants = malloc(sizeof(PettyValue) * vm.ConstantsCount);
    for (uint32_t i = 0; i < vm.ConstantsCount; i++) 
        VM_addConstant(i);
    vm.stack = stack_init();
    VM_initBuiltins();
    return 0;
}

uint64_t VM_read_ID() {
    uint64_t v = 0;

    for (int i = 0; i < 8; i++) 
    {
        v |= ((uint8_t)current_code) << (8 * i);
        vm_advance;
    }

    return v;
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
            case PUSH_CONSTANT :  
            {
                int32_t index = VM_read_raw_Int();
                vm_push(vm.Constants[index]);
                break;
            }

            case STORE_GLOBAL : 
            {
                uint64_t index = VM_read_ID();
                PettyValue obj = stack_pop(&vm.stack);
                vm.Globals[index] = obj;
                break;
            }

            case LOAD_GLOBAL : 
            {
                uint64_t index = VM_read_ID();
                vm_push(vm.Globals[index]);
                break;
            }

            case ADD_INT : 
            {
                PettyValue pt_val;
                int32_t right = vm_pop.as.as_int;
                int32_t left = vm_pop.as.as_int;
                pt_val.as.as_int = left + right;
                pt_val.kind = PT_INT32;
                stack_push(&vm.stack, pt_val);
                break;
            }

            case SUB_INT : {
                    PettyValue pt_val;
                    int32_t right = vm_pop.as.as_int;
                    int32_t left = vm_pop.as.as_int;
                    pt_val.kind = PT_INT32;
                    pt_val.as.as_int = left - right;
                    stack_push(&vm.stack, pt_val);
                break;
            }

            case MUL_INT : {
                PettyValue result;
                result.kind = PT_INT32;
                int32_t right = vm_pop.as.as_int;
                int32_t left = vm_pop.as.as_int;
                result.as.as_int = left * right;
                stack_push(&vm.stack, result);
                break;
            }
        }
    }

    printf("%d\n", vm.Globals[6].as.as_int);
    printf("%d\n", vm.Globals[7].as.as_int);
    VM_free();
    
    return 0;
}