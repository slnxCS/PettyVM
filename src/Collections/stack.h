#ifndef STACK_H
#define STACK_H

#include "../Value/PettyValue.h"

typedef struct stack {
    int32_t capacity;
    uint64_t top_index;
    PettyValue* ptr;
} stack;

extern stack stack_init();
extern void stack_push(stack* s, PettyValue value);
extern PettyValue stack_pop(stack* s);

#define stack_peek(stack_ptr) (stack_ptr->ptr[stack_ptr->top_index - 1])

#endif // STACK_H