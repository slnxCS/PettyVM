#include "PettyObject.hpp"
#include "PettyClass.hpp"
#include "VM/VM.hpp"
#include "Value/PettyValue.hpp"

namespace Petty {
    const PettyClass* ClassOf(VM* vm, const PettyValue& val) {
        switch (val.kind)
        {
            case PettyValue::Kind::PT_OBJ_PTR :
                return &(vm->Classes()[val.as.as_obj_ptr->Class_ID]);
        
            case PettyValue::Kind::PT_INT32 :
                return &(vm->Classes()[INT32_CLASS_ID]);

            case PettyValue::Kind::PT_FLOAT32 : 
                return &(vm->Classes()[FLOAT32_CLASS_ID]);

            case PettyValue::Kind::PT_BOOL : 
                return &(vm->Classes()[BOOL_CLASS_ID]);
        }
        return nullptr;
    }

    [[noreturn]] void ThrowCannotCastException(uint32_t id_from, uint32_t id_to) {
        fprintf(stderr, "Fatal error : Cannot convert object with type %d to type %d", id_from , id_to);
        exit(17);
    }

    void ThrowIfNotMatchClasses(VM* vm, const PettyValue& val, uint32_t classID) {
        uint32_t objClassId = ClassOf(vm, val)->ID;
        if (objClassId != classID)
            ThrowCannotCastException(objClassId, classID);
    }
}