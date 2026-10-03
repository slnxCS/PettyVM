#ifndef PETTYCLASS_H
#define PETTYCLASS_H

#include "../../types.h"

typedef struct PettyClass {
    int32_t ID;
    int32_t DerivedID;
    uint32_t Fields_Count;
    char* Name;
    uint32_t NameLength;
    uint64_t* VirtualMethodsTable;
} PettyClass;

#endif // PETTYCLASS_H