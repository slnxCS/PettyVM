#ifndef PETTYCLASS_H
#define PETTYCLASS_H

#include "../../types.h"
#include "../PettyValue.h"

typedef struct VM VM;

#define INT32_CLASS_ID (2)
#define FLOAT32_CLASS_ID (3)
#define BOOL_CLASS_ID (6)

typedef struct PettyClass {
    int32_t ID;
    int32_t DerivedID;
    uint32_t Fields_Count;
    char* Name;
    uint32_t NameLength;
    uint64_t* VirtualMethodsTable;
} PettyClass;

inline extern PettyClass* ClassOf(VM* vm, PettyValue val);

#endif // PETTYCLASS_H