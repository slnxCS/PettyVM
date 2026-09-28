#ifndef STACK_H
#define STACK_H

#include "../Value/PettyValue.h"

typedef struct stack {
    int capacity;
    int top_index;
    PettyValue* ptr;
} stack;

extern stack stack_init();
extern void stack_push(stack* s, PettyValue value);
extern PettyValue stack_pop(stack* s);

#endif // STACK_H