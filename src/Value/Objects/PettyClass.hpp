#ifndef PETTYCLASS_H
#define PETTYCLASS_H

#include "Value/PettyValue.hpp"
#include "VM/VM.hpp"

#define INT32_CLASS_ID (2)
#define FLOAT32_CLASS_ID (3)
#define BOOL_CLASS_ID (6)

namespace Petty {
    struct PettyClass {
        int32_t ID;
        int32_t DerivedID;
        uint32_t Fields_Count;
        char* Name;
        uint32_t NameLength;
        uint64_t* VirtualMethodsTable;
    };

    const PettyClass* ClassOf(VM* vm, const PettyValue& val);
    void ThrowIfNotMatchClasses(VM* vm, const PettyValue& val, uint32_t classID);
}

#endif // PETTYCLASS_H