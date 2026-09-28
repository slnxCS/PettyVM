#ifndef PT_VAL_H
#define PT_VAL_H

#include "./Objects/PettyObject.h"
#include "../types.h"


typedef enum PettyValueKind : byte {
    PT_INT32,
    PT_FLOAT32,
    PT_BOOL,
    PT_OBJ_REF,
} PettyValueKind;

typedef struct PettyValue {
    PettyValueKind kind;
    union {
        int32_t as_int;
        float32_t as_float;
        bool as_bool;
        PettyObject* as_obj_ref;
    } as;
} PettyValue;

#endif // PT_VAL_H