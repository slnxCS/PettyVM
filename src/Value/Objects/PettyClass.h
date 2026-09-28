#ifndef PETTYCLASS_H
#define PETTYCLASS_H

#include "../../types.h"

typedef struct PettyClass {
    int32_t ID;
    int32_t DerivedID;
    char* Name;
    uint32_t NameLength;
} PettyClass;

#endif // PETTYCLASS_H