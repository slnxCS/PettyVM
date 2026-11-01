#ifndef PT_OBJ_H
#define PT_OBJ_H

#include "PettyClass.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct PettyValue PettyValue;

typedef struct PettyObject {
    uint32_t Class_ID;
    uint32_t Fields_Count;
    PettyValue* Fields;
    bool is_free;
} PettyObject;

inline extern void ThrowOutOfMemoryException();

#endif // PT_OBJ_H