#ifndef PT_VAL_H
#define PT_VAL_H

#include "PettyObject.h"
#include <stdint.h>


typedef enum PettyValueKind {
    PT_INT32,
    PT_OBJ_REF,
} PettyValueKind;

typedef struct PettyValue {
    PettyValueKind kind;
    union {
        int32_t as_int;
        PettyObject* as_obj_ref;
    } as;
} PettyValue;

#endif // PT_VAL_H