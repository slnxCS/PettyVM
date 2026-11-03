#ifndef VM_H
#define VM_H

#include "Value/Objects/PettyObject.hpp"
#include "Value/PettyValue.hpp"
#include "Collections/stack.hpp"
#include <cstdint>

#define BYTECODE_VER (1.395f)

#define FRAME_STACK_MAX (500001)
#define HEAP_MEMORY_OBJ_COUNT (500000)
#define HEAP_GET_ALLOC_COUNT_BYTES (HEAP_MEMORY_OBJ_COUNT * sizeof(PettyObject*))

namespace Petty {
    enum class OpCode : uint8_t {
        PUSH_CONSTANT = 1,
        STORE_LOCAL = 2,
        STORE_GLOBAL = 3,
        CALL_NATIVE = 4,
        LOAD_LOCAL = 5,
        LOAD_GLOBAL = 6,
        RET = 7,
        CALL = 8,
        RESERVE_LOCAL = 9,

        ADD_INT = 10,
        SUB_INT = 11,
        DIV_INT = 12,
        MUL_INT = 13,
        CAST_FROM_FLOAT32_TO_INT32 = 14,
        HALT = 15,
        CAST_FROM_INT32_TO_FLOAT32 = 16,
        ADD_FLOAT = 17,
        SUB_FLOAT = 18,
        MUL_FLOAT = 19,
        DIV_FLOAT = 20,
        JMP_IF_FALSE = 26,
        JMP_IF_TRUE = 27,
        JMP = 28,

        ALLOC_OBJ = 25,
        CALL_METHOD = 30,
        LOAD_FIELD = 31,
        STORE_FIELD = 32,
        ALLOC_ARR = 35,
        ARR_GET = 36,
        ARR_SET = 37,

        INT_EQ = 29,
        INT_NEQ = 38,
        INT_NEG = 39,
        INT_LEQ = 40,
        INT_GEQ = 41,
        INT_LE = 42,
        INT_GE = 43,

        FLOAT_EQ = 44,
        FLOAT_LEQ = 45,
        FLOAT_GEQ = 46,
        FLOAT_LE = 47,
        FLOAT_GE = 48,
        FLOAT_NEQ = 54,
        FLOAT_NEG = 55,

        BOOL_EQ = 49,
        BOOL_NOT = 50,
        BOOL_AND = 51,
        BOOL_OR = 52,
        BOOL_NEQ = 53,
    };

    enum VM_ConstantType {
        CONSTANT_INT = 0,
        CONSTANT_FLOAT = 1,
        CONSTANT_BOOL = 2,
    };

    struct Frame {
        uint32_t return_ip;
        int32_t stack_ptr_index;
        uint32_t arity;
        uint32_t locals_count;
    };

    class PettyClass;

    class VM
    {
        private :
            uint8_t* OpCodes;
            uint64_t current_instruction;
            Stack<PettyValue> stack;
            PettyValue* Globals;
            PettyValue* Constants;
            uint32_t ConstantsCount;
            uint64_t* Functions;
            Frame call_stack[FRAME_STACK_MAX];
            int32_t frame_pointer;
            uint64_t heap_size;
            PettyObject** heap;
            uint32_t classes_count;
            PettyClass* classes;
            const char* fileName;

            void addConstant(uint32_t constant_index);
            void initFuncs(int32_t count);
            void initHeap();
            void initClasses();
            void FreeObj(PettyObject* obj);
            void FreeHeap();
            void FreeClassTable();
            int Free();
        public : 
            VM(uint8_t* opcodes, const char* fileName) {
                OpCodes = opcodes;
                this->fileName = fileName;
                Globals = nullptr;
                Constants = nullptr;
                Functions = nullptr;
                heap = nullptr;
                heap_size = 0;
                classes = nullptr;
                classes_count = 0;
            }

            ~VM() {
                Free();
            }

            int32_t ReadInt();
            _Float32 ReadFloat();
            bool ReadBool();
            void ReadString(char* buffer, uint32_t length);
            int Init();
            int Start();

            const PettyClass* Classes() const { return classes; }
            Stack<PettyValue>* GetStack() { return &stack; }
    };
}

#endif // VM_H