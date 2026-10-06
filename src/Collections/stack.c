#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

#define STACK_GET_REALLOC_C(old) (((old) < STACK_C_MAX / 2) ? (old) * 2 : STACK_C_MAX)
#define STACK_C_MAX 100000
#define STACK_C_START 64

void ensure_ptr(void* ptr) 
{
    if (ptr == NULL) {
        fprintf(stderr, "Stack memory allocation error\n");
        exit(1);
    }
}

stack stack_init() 
{
    stack s = {.capacity = STACK_C_START, .top_index = 0, .ptr = malloc(STACK_C_START * sizeof(PettyValue))};
    ensure_ptr(s.ptr);
    return s;
}

void stack_push(stack* s, PettyValue value) 
{
    if (s->top_index >= s->capacity) {
        if (s->capacity >= STACK_C_MAX) {
            fprintf(stderr, "Stack overflow\n");
            exit(1);
        }

        s->capacity = STACK_GET_REALLOC_C(s->capacity);

        s->ptr = realloc(s->ptr, s->capacity * sizeof(PettyValue));

        ensure_ptr(s->ptr);
    }

    s->ptr[s->top_index] = value;
    s->top_index++;
}

PettyValue stack_pop(stack* s) 
{
    if (s->top_index <= 0)
    {
        fprintf(stderr, "stack_pop error : Stack top index <= 0");
        exit(1);
    }

    s->top_index--;
    volatile PettyValue val = s->ptr[s->top_index];
    return val;
}