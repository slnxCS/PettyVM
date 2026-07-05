#include "VM.h"
#include "Collections/stack.h"
#include "PettyValue.h"
#define current_code vm.OpCodes[vm.current_instruction]
#define vm_advance (vm.current_instruction++)
#define vm_pop (stack_pop(&vm.stack))

VM vm;

uint64_t VM_read_ID() {
    uint64_t v = 0;

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

int VM_Start(unsigned char* compiled, int c_len) 
{
    vm.current_instruction = 0;
    vm.OpCodes = compiled;
    vm.OpLenght = c_len;
    vm.stack = stack_init();
    for (; vm.current_instruction < vm.OpLenght; vm_advance) {
        switch (current_code) {
            case PUSH_INT : 
                stack_push(&vm.stack, VM_read_Int());
            break;

            case STORE_GLOBAL : 
            {
                uint64_t index = VM_read_ID();
                PettyValue obj = stack_pop(&vm.stack);
                // проблема тут
                
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
        }
    }
    return 0;
}