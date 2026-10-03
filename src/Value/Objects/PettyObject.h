#ifndef PT_OBJ_H
#define PT_OBJ_H

#include "PettyClass.h"

typedef struct PettyValue PettyValue;

typedef struct PettyObject {
    uint32_t Class_ID;
    uint32_t Fields_Count;
    PettyValue* Fields;
    bool is_free;
} PettyObject;

#endif // PT_OBJ_H