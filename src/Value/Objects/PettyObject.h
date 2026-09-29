#ifndef PT_OBJ_H
#define PT_OBJ_H

#include "PettyClass.h"
#include "../PettyValue.h"

typedef struct PettyObject {
    uint32_t ID;
    uint32_t Fields_Count;
    PettyValue* Fields;
    bool is_free;
} PettyObject;

#endif // PT_OBJ_H