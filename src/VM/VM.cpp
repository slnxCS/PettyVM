#include "VM.hpp"
#include "Collections/stack.hpp"
#include "Value/PettyValue.hpp"
#include "Value/Objects/PettyClass.hpp"
#include "Value/Objects/PettyObject.hpp"
#include "Value/ArrayTools.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>
#include <algorithm>
#include <new>

#undef USELABELS

#define DEBUG
#ifdef __GNUC__
    #ifndef DEBUG
        #define USELABELS
    #endif
#endif

#undef CASE
#undef DISPATCH
#undef DEFAULT

#ifdef USELABELS
    #define CASE(opcode) opc_ ## opcode
    #define DISPATCH(vm) goto *dispatch_table[CURRENT_CODE(vm)]
    #define DEFAULT opc_default
#else 
    #define CASE(opcode) case (uint8_t)OpCode::opcode
    #define DISPATCH(vm) break
    #define DEFAULT default
#endif

#define CURRENT_CODE(vm) (vm->OpCodes[vm->current_instruction])
#define VM_ADVANCE(vm) (vm->current_instruction++)
#define VM_ADVANCE_N(vm, len) vm->current_instruction += len;
#define VM_POP(vm) (vm->stack.Pop())
#define VM_PUSH(vm, obj) vm->stack.Push(obj)
#define VM_PEEK(vm) (vm->stack.Peek())

#define _processOpCodeMath(vm, type, typeAs, opCode, operator, returnTypeAs, returnTypeKind, typeCast) \
CASE(opCode) : \
{\
    VM_ADVANCE(vm);\
    PettyValue pt_val;\
    type right = VM_POP(vm).as.typeAs;\
    type left = VM_POP(vm).as.typeAs;\
    pt_val.kind = returnTypeKind;\
    pt_val.as.returnTypeAs = (typeCast)left operator (typeCast)right;\
    VM_PUSH(vm, pt_val);\
    DISPATCH(vm);\
}

#define _processOpCodeMathUnary(vm, type, typeAs, opCode, operator, returnTypeAs, returnTypeKind, typeCast) \
CASE(opCode) : \
{\
    VM_ADVANCE(vm);\
    PettyValue pt_val;\
    type right = VM_POP(vm).as.typeAs;\
    pt_val.kind = returnTypeKind;\
    pt_val.as.returnTypeAs = operator((typeCast)right);\
    VM_PUSH(vm, pt_val);\
    DISPATCH(vm);\
}

#define _processOpCodeCast(vm, opCode, fromAsType, toAsType, castKind, castType) \
CASE(opCode) : \
{\
    VM_ADVANCE(vm);\
    PettyValue val = VM_POP(vm);\
    val.as.toAsType = (castType)val.as.fromAsType;\
    val.kind = castKind;\
    VM_PUSH(vm, val);\
    DISPATCH(vm);\
}

using PettyNativeFunc = void (*)(Petty::VM* vm, int32_t arity);
using Kind = Petty::PettyValue::Kind;

namespace Petty {
    void pt_vm_native_print_int32(VM* vm, int32_t arity) {
        for (int i = 0; i < arity; i++) 
        {
            int32_t num = vm->GetStack()->Pop().as.as_int;
            printf("%d\n", num);
        }
    }

    void pt_vm_native_print_float32(VM* vm, int32_t) {
        _Float32 num = vm->GetStack()->Pop().as.as_float;
        printf("%g\n", (double)num);
    }

    void pt_vm_native_read_int32(VM* vm, int32_t) {
        PettyValue val;
        val.kind = PettyValue::Kind::PT_INT32;
        scanf("%d", &(val.as.as_int));
        vm->GetStack()->Push(val);
    }

    void pt_vm_native_random(VM* vm, int32_t) {
        int32_t max = vm->GetStack()->Pop().as.as_int;
        int32_t min = vm->GetStack()->Pop().as.as_int;
        PettyValue val;
        val.kind = PettyValue::Kind::PT_INT32;
        val.as.as_int = min + rand() % (max - min + 1);
        vm->GetStack()->Push(val);
    }

    void pt_vm_native_print_bool(VM* vm, int32_t) {
        PettyValue value = vm->GetStack()->Pop();
        printf("%s\n", value.as.as_bool ? "true" : "false");
    }

    void pt_vm_native_arrayget(VM* vm, int32_t) {
        uint32_t index = vm->GetStack()->Pop().as.as_int;
        PettyObject* array = vm->GetStack()->Pop().as.as_obj_ptr;
        Array_ThrowIfOutOfRange(array, index);
        vm->GetStack()->Push((PTArrayGet(array, index)));
    }

    void pt_vm_native_arrayset(VM* vm, int32_t) {
        PettyValue val = vm->GetStack()->Pop();
        uint32_t index = vm->GetStack()->Pop().as.as_int;
        PettyObject* array = vm->GetStack()->Pop().as.as_obj_ptr;
        Array_ThrowIfOutOfRange(array, index);
        PTArraySet(array, index, val);
    }

    void pt_vm_native_arraylen(VM* vm, int32_t) {
        vm->GetStack()->Push({(int32_t)PTArrayLen(vm->GetStack()->Pop().as.as_obj_ptr)});
    }

    PettyNativeFunc vm_native_funcs[] = {
        pt_vm_native_print_int32,
        pt_vm_native_print_float32,
        pt_vm_native_print_bool,
        pt_vm_native_read_int32,
        pt_vm_native_random,
        pt_vm_native_arrayget,
        pt_vm_native_arrayset,
        pt_vm_native_arraylen,
    };

    void VM::addConstant(uint32_t constant_index) {
        unsigned char type = CURRENT_CODE(this);
        VM_ADVANCE(this);
        switch (type) {
            case CONSTANT_INT :
            {
                PettyValue num = ReadInt();
                Constants[constant_index] = num;
                break;
            }

            case CONSTANT_FLOAT : 
            {
                PettyValue num = ReadFloat();
                Constants[constant_index] = num;
                break;
            }

            case CONSTANT_BOOL : 
            {
                PettyValue val = ReadBool();
                Constants[constant_index] = val;
                break;
            }
        }
    }

    void VM::initFuncs(int32_t count) {
        for (int i = 0; i < count; i++) {
            uint32_t len = ReadInt();
            Functions[i] = current_instruction;
            VM_ADVANCE_N(this, len);
        }
    }

    void VM::initHeap() {
        heap_size = HEAP_MEMORY_OBJ_COUNT;
        heap = new (std::nothrow) PettyObject* [heap_size];
        for (uint64_t i = 0; i < heap_size; i++) {
            heap[i] = NULL;
        }
    }

    void VM::initClasses() {
        for (uint32_t i = 0; i < classes_count; i++) {
            PettyClass* _class = &classes[i];
            _class->NameLength = ReadInt();
            _class->Name = new char[_class->NameLength + 1];
            if (!_class->Name) {
                fprintf(stderr, "Malloc error : failed to allocate %d bytes\n", _class->NameLength + 1);
                exit(1);
            }
            ReadString(_class->Name, _class->NameLength);
            _class->Name[_class->NameLength] = '\0';
            _class->ID = ReadInt();
            _class->DerivedID = ReadInt();
            _class->Fields_Count = ReadInt();
            uint32_t vmethods_c = ReadInt();
            _class->VirtualMethodsTable = new uint64_t[vmethods_c];
            if (!_class->VirtualMethodsTable) {
                fprintf(stderr, "Malloc error : failed to allocate %d bytes\n", vmethods_c * 8);
                exit(1);
            }
            for (uint32_t j = 0; j < vmethods_c; j++) {
                uint32_t func_len = ReadInt();
                uint64_t func_index = current_instruction;
                _class->VirtualMethodsTable[j] = func_index;
                VM_ADVANCE_N(this, func_len);
            }
        }
    }

    int VM::Init()
    {
        current_instruction = 0;
        frame_pointer = 0;
        char magic[7];
        strncpy(magic, (char*)OpCodes, 6);
        magic[6] = '\0';

        if (memcmp(magic, "[PTVM]", 6) != 0) 
        {
            fprintf(stderr, "File '%s' is not PettyLang bytecode. Terminating VM\n", fileName);
            exit(7);
        }
        VM_ADVANCE_N(this, 6);
        _Float32 file_byteCodeVersion = ReadFloat();
        if (file_byteCodeVersion != BYTECODE_VER) {
            fprintf(stderr, "The bytecode version for this file is %s. Current VM bytecode version : %g. File bytecode version : %g. \n" 
                "Please recompile the file to match the current bytecode version of the vm, or update the vm for your file version\n", 
                    file_byteCodeVersion > BYTECODE_VER ? "newer" : "outdated", (double)BYTECODE_VER, (double)file_byteCodeVersion);
            exit(2);
        }

        initHeap();

        ConstantsCount = ReadInt();
        Constants = new PettyValue[ConstantsCount];
        for (uint32_t i = 0; i < ConstantsCount; i++) 
            addConstant(i);
        
        uint32_t funcs_c = ReadInt();
        Functions = new uint64_t[funcs_c];
        initFuncs(funcs_c);
        classes_count = ReadInt();
        classes = new PettyClass[classes_count];
        initClasses();

        int32_t globals_c = ReadInt();
        Globals = new PettyValue[globals_c];
        Start(); // Initialize globals
        VM_ADVANCE(this);
        return 0;
    }

    //uint64_t VM_read_ID() {
    //    uint64_t v = 0;
    //
    //    for (int i = 0; i < 8; i++) 
    //    {
    //        v |= ((uint8_t)CURRENT_CODE) << (8 * i);
    //        VM_ADVANCE;
    //    }
    //
    //    return v;
    //}

    bool VM::ReadBool() {
        bool b = CURRENT_CODE(this) != 0;
        VM_ADVANCE(this);
        return b;
    }

    _Float32 VM::ReadFloat() {
        _Float32 num;
        memcpy(&num, OpCodes + current_instruction, 4);
        VM_ADVANCE_N(this, 4);
        return num;
    }

    void VM::ReadString(char* buffer, uint32_t length) {
        for (uint32_t i = 0; i < length; i++, VM_ADVANCE(this)) {
            buffer[i] = (char)CURRENT_CODE(this);
        }
    }

    inline int32_t VM::ReadInt() {
        int32_t val;
        memcpy(&val, OpCodes + current_instruction, 4);
        VM_ADVANCE_N(this, 4);
        return val;
    }

    void VM::FreeObj(PettyObject* obj) {
        if (!obj) return;

        delete[] obj->Fields;
        delete obj;
    }

    void VM::FreeHeap() {
        for (uint64_t i = 0; i < heap_size; i++) {
            if (heap[i]) FreeObj(heap[i]);
        }

        delete[] heap;
    }

    void VM::FreeClassTable() {
        for (uint32_t i = 0; i < classes_count; i++) {
            delete[] classes[i].Name;
            delete[] classes[i].VirtualMethodsTable;
        }
        delete[] classes;
    }

    int VM::Free() {
        FreeHeap();
        FreeClassTable();

        delete[] Functions;
        delete[] Constants;
        delete[] Globals;
        delete[] OpCodes;
        return 0;
    }

    uint64_t find_free_memory_heap(PettyObject** heap, uint64_t heap_len) {
        for (uint64_t i = 0; i < heap_len; i++) {
            if (heap[i] == NULL || heap[i]->is_free) return i;
        }
        return -1;
    }

    int VM::Start() 
    {
#ifdef USELABELS
        // Table is indexed by the OpCode value, so it can't get out of sync with the enum
        static const void* dispatch_table[256];
        static bool dispatch_table_ready = false;
        if (!dispatch_table_ready) {
            for (auto& entry : dispatch_table) entry = &&DEFAULT;
            dispatch_table[(uint8_t)OpCode::PUSH_CONSTANT] = &&CASE(PUSH_CONSTANT);
            dispatch_table[(uint8_t)OpCode::STORE_LOCAL] = &&CASE(STORE_LOCAL);
            dispatch_table[(uint8_t)OpCode::STORE_GLOBAL] = &&CASE(STORE_GLOBAL);
            dispatch_table[(uint8_t)OpCode::CALL_NATIVE] = &&CASE(CALL_NATIVE);
            dispatch_table[(uint8_t)OpCode::LOAD_LOCAL] = &&CASE(LOAD_LOCAL);
            dispatch_table[(uint8_t)OpCode::LOAD_GLOBAL] = &&CASE(LOAD_GLOBAL);
            dispatch_table[(uint8_t)OpCode::RET] = &&CASE(RET);
            dispatch_table[(uint8_t)OpCode::CALL] = &&CASE(CALL);
            dispatch_table[(uint8_t)OpCode::RESERVE_LOCAL] = &&CASE(RESERVE_LOCAL);
            dispatch_table[(uint8_t)OpCode::ADD_INT] = &&CASE(ADD_INT);
            dispatch_table[(uint8_t)OpCode::SUB_INT] = &&CASE(SUB_INT);
            dispatch_table[(uint8_t)OpCode::DIV_INT] = &&CASE(DIV_INT);
            dispatch_table[(uint8_t)OpCode::MUL_INT] = &&CASE(MUL_INT);
            dispatch_table[(uint8_t)OpCode::CAST_FROM_FLOAT32_TO_INT32] = &&CASE(CAST_FROM_FLOAT32_TO_INT32);
            dispatch_table[(uint8_t)OpCode::HALT] = &&CASE(HALT);
            dispatch_table[(uint8_t)OpCode::CAST_FROM_INT32_TO_FLOAT32] = &&CASE(CAST_FROM_INT32_TO_FLOAT32);
            dispatch_table[(uint8_t)OpCode::ADD_FLOAT] = &&CASE(ADD_FLOAT);
            dispatch_table[(uint8_t)OpCode::SUB_FLOAT] = &&CASE(SUB_FLOAT);
            dispatch_table[(uint8_t)OpCode::MUL_FLOAT] = &&CASE(MUL_FLOAT);
            dispatch_table[(uint8_t)OpCode::DIV_FLOAT] = &&CASE(DIV_FLOAT);
            dispatch_table[(uint8_t)OpCode::JMP_IF_FALSE] = &&CASE(JMP_IF_FALSE);
            dispatch_table[(uint8_t)OpCode::JMP_IF_TRUE] = &&CASE(JMP_IF_TRUE);
            dispatch_table[(uint8_t)OpCode::JMP] = &&CASE(JMP);
            dispatch_table[(uint8_t)OpCode::ALLOC_OBJ] = &&CASE(ALLOC_OBJ);
            dispatch_table[(uint8_t)OpCode::CALL_METHOD] = &&CASE(CALL_METHOD);
            dispatch_table[(uint8_t)OpCode::LOAD_FIELD] = &&CASE(LOAD_FIELD);
            dispatch_table[(uint8_t)OpCode::STORE_FIELD] = &&CASE(STORE_FIELD);
            dispatch_table[(uint8_t)OpCode::ALLOC_ARR] = &&CASE(ALLOC_ARR);
            dispatch_table[(uint8_t)OpCode::ARR_GET] = &&CASE(ARR_GET);
            dispatch_table[(uint8_t)OpCode::ARR_SET] = &&CASE(ARR_SET);
            dispatch_table[(uint8_t)OpCode::INT_EQ] = &&CASE(INT_EQ);
            dispatch_table[(uint8_t)OpCode::INT_NEQ] = &&CASE(INT_NEQ);
            dispatch_table[(uint8_t)OpCode::INT_NEG] = &&CASE(INT_NEG);
            dispatch_table[(uint8_t)OpCode::INT_LEQ] = &&CASE(INT_LEQ);
            dispatch_table[(uint8_t)OpCode::INT_GEQ] = &&CASE(INT_GEQ);
            dispatch_table[(uint8_t)OpCode::INT_LE] = &&CASE(INT_LE);
            dispatch_table[(uint8_t)OpCode::INT_GE] = &&CASE(INT_GE);
            dispatch_table[(uint8_t)OpCode::FLOAT_EQ] = &&CASE(FLOAT_EQ);
            dispatch_table[(uint8_t)OpCode::FLOAT_LEQ] = &&CASE(FLOAT_LEQ);
            dispatch_table[(uint8_t)OpCode::FLOAT_GEQ] = &&CASE(FLOAT_GEQ);
            dispatch_table[(uint8_t)OpCode::FLOAT_LE] = &&CASE(FLOAT_LE);
            dispatch_table[(uint8_t)OpCode::FLOAT_GE] = &&CASE(FLOAT_GE);
            dispatch_table[(uint8_t)OpCode::FLOAT_NEQ] = &&CASE(FLOAT_NEQ);
            dispatch_table[(uint8_t)OpCode::FLOAT_NEG] = &&CASE(FLOAT_NEG);
            dispatch_table[(uint8_t)OpCode::BOOL_EQ] = &&CASE(BOOL_EQ);
            dispatch_table[(uint8_t)OpCode::BOOL_NOT] = &&CASE(BOOL_NOT);
            dispatch_table[(uint8_t)OpCode::BOOL_AND] = &&CASE(BOOL_AND);
            dispatch_table[(uint8_t)OpCode::BOOL_OR] = &&CASE(BOOL_OR);
            dispatch_table[(uint8_t)OpCode::BOOL_NEQ] = &&CASE(BOOL_NEQ);
            dispatch_table_ready = true;
        }

        DISPATCH(this);
    #else
        while (1) {
        switch (CURRENT_CODE(this)) {
    #endif

        CASE(RET) : {
                VM_ADVANCE(this);
                Frame current_frame = call_stack[--frame_pointer];
                if (stack.TopIndex > current_frame.stack_ptr_index + current_frame.arity + current_frame.locals_count) {
                    PettyValue res = VM_POP(this);
                    stack.TopIndex = current_frame.stack_ptr_index;
                    VM_PUSH(this, res);
                }
                else 
                    stack.TopIndex = current_frame.stack_ptr_index;
                current_instruction = current_frame.return_ip;
                DISPATCH(this);
            }

            CASE(CALL_METHOD) : {
                VM_ADVANCE(this);
                if (frame_pointer >= FRAME_STACK_MAX) 
                {
                    fprintf(stderr, "Call Stack overflow error!");
                    exit(5);
                }

                int32_t func_index = ReadInt();
                int32_t func_arity = ReadInt();
                PettyValue instance = stack[stack.TopIndex - func_arity];
                Frame frame;
                frame.return_ip = current_instruction;
                frame.stack_ptr_index = stack.TopIndex - func_arity;
                frame.arity = func_arity;
                call_stack[frame_pointer++] = frame;
                current_instruction = ClassOf(this, instance)->VirtualMethodsTable[func_index];
                DISPATCH(this);
            }

            CASE(CALL) : 
            {
                VM_ADVANCE(this);
                if (frame_pointer >= FRAME_STACK_MAX) 
                {
                    fprintf(stderr, "Call Stack overflow error!");
                    exit(5);
                }
                int32_t func_index = ReadInt();
                int32_t func_arity = ReadInt();
                Frame frame;
                frame.return_ip = current_instruction;
                frame.stack_ptr_index = stack.TopIndex - func_arity;
                frame.arity = func_arity;
                call_stack[frame_pointer++] = frame;
                current_instruction = Functions[func_index];
                DISPATCH(this);
            }

            CASE(RESERVE_LOCAL) : 
            {
                VM_ADVANCE(this);
                int32_t count = ReadInt();
                int32_t locals_count = count - call_stack[frame_pointer - 1].arity;
                call_stack[frame_pointer - 1].locals_count = locals_count;
                stack.TopIndex += locals_count;
                DISPATCH(this);
            }

            CASE(CALL_NATIVE) :
            {
                VM_ADVANCE(this);
                int32_t native_index = ReadInt();
                int32_t arity = ReadInt();
                vm_native_funcs[native_index](this, arity);
                DISPATCH(this);
            }

            CASE(POP) : {
                VM_ADVANCE(this);
                stack.Pop();
                DISPATCH(this);
            }

            CASE(PUSH_CONSTANT) :  
            {
                VM_ADVANCE(this);
                int32_t index = ReadInt();
                VM_PUSH(this, Constants[index]);
                DISPATCH(this);
            }

            CASE(ALLOC_OBJ) : 
            {
                VM_ADVANCE(this);
                uint32_t class_id = ReadInt();
                uint64_t free_index = find_free_memory_heap(heap, heap_size);
                if (free_index == (uint64_t)-1) ThrowOutOfMemoryException();
                PettyObject* obj = heap[free_index];

                if (!obj) {
                    obj = new PettyObject{};
                    heap[free_index] = obj;
                }

                obj->is_free = false;
                obj->Class_ID = class_id;
                obj->Fields_Count = classes[class_id].Fields_Count;
                delete[] obj->Fields;
                obj->Fields = new PettyValue[obj->Fields_Count];

                VM_PUSH(this, {obj}); 
                DISPATCH(this);
            }

            CASE(ALLOC_ARR) : {
                VM_ADVANCE(this);
                static const uint32_t class_id = ARRAY_CLASS_ID;
                uint32_t length = VM_POP(this).as.as_int;
                uint64_t free_index = find_free_memory_heap(heap, heap_size);
                if (free_index == (uint64_t)-1) {
                    ThrowOutOfMemoryException();
                }
                PettyObject* obj = heap[free_index];

                if (!obj) {
                    obj = new PettyObject{};
                    heap[free_index] = obj;
                }

                obj->is_free = false;
                obj->Class_ID = class_id;
                obj->Fields_Count = length;
                delete[] obj->Fields;
                obj->Fields = new PettyValue[length];

                VM_PUSH(this, {obj}); 
                DISPATCH(this);
            }

            CASE(ARR_GET) : {
                VM_ADVANCE(this);
                PettyObject* obj = VM_POP(this).as.as_obj_ptr;
                uint32_t index = VM_POP(this).as.as_int;
                Array_ThrowIfOutOfRange(obj, index);
                VM_PUSH(this, PTArrayGet(obj, index));
                DISPATCH(this);
            }

            CASE(ARR_SET) : {
                VM_ADVANCE(this);
                PettyObject* obj = VM_POP(this).as.as_obj_ptr;
                uint32_t index = VM_POP(this).as.as_int;
                Array_ThrowIfOutOfRange(obj, index);
                PettyValue val = VM_POP(this);
                PTArraySet(obj, index, val);
                DISPATCH(this);
            }

            CASE(LOAD_FIELD) : {
                VM_ADVANCE(this);
                PettyObject* obj = VM_POP(this).as.as_obj_ptr;
                uint32_t index = ReadInt();
                VM_PUSH(this, obj->Fields[index]);
                DISPATCH(this);
            }

            CASE(STORE_FIELD) : {
                VM_ADVANCE(this);
                PettyObject* obj = VM_POP(this).as.as_obj_ptr;
                PettyValue val = VM_POP(this);
                uint32_t field_index = ReadInt();
                obj->Fields[field_index] = val;
                DISPATCH(this);
            }

            CASE (CHECK_CAST_CLASSES) : {
                VM_ADVANCE(this);
                uint32_t class_id = ReadInt();
                PettyValue val = VM_POP(this);
                ThrowIfNotMatchClasses(this, val, class_id);
                DISPATCH(this);
            }

            CASE(STORE_GLOBAL) : 
            {
                VM_ADVANCE(this);
                int32_t index = ReadInt();
                PettyValue val = VM_POP(this);
                Globals[index] = val;
                DISPATCH(this);
            }

            CASE(STORE_LOCAL) : 
            {  
                VM_ADVANCE(this);
                int32_t index = ReadInt();
                PettyValue val = VM_POP(this);
                stack[call_stack[frame_pointer - 1].stack_ptr_index + index] = val;
                DISPATCH(this);
            }

            CASE(LOAD_LOCAL) : 
            {  
                VM_ADVANCE(this);
                int32_t index = ReadInt();
                VM_PUSH(this, stack[call_stack[frame_pointer - 1].stack_ptr_index + index]);
                DISPATCH(this);
            }

            CASE(LOAD_GLOBAL) : 
            {
                VM_ADVANCE(this);
                int32_t index = ReadInt();
                VM_PUSH(this, Globals[index]);
                DISPATCH(this);
            }

            CASE(JMP) : 
            {
                VM_ADVANCE(this);
                current_instruction = ReadInt();
                DISPATCH(this);
            }

            CASE(JMP_IF_FALSE) : 
            {
                VM_ADVANCE(this);
                PettyValue val = VM_POP(this);
                int32_t pos = ReadInt();
                if (!val.as.as_bool) 
                {
                    current_instruction = pos;
                }
                DISPATCH(this);
            }

            CASE(JMP_IF_TRUE) : 
            {
                VM_ADVANCE(this);
                PettyValue val = VM_POP(this);
                int32_t pos = ReadInt();
                if (val.as.as_bool) 
                {
                    current_instruction = pos;
                }
                DISPATCH(this);
            }

            _processOpCodeCast(this, CAST_FROM_FLOAT32_TO_INT32, as_float, as_int, Kind::PT_INT32, int32_t)
            _processOpCodeCast(this, CAST_FROM_INT32_TO_FLOAT32, as_int, as_float, Kind::PT_FLOAT32, _Float32)

            _processOpCodeMath(this, int32_t, as_int, INT_EQ, ==, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, INT_NEQ, !=, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, INT_LE, <, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, INT_GE, >, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, INT_LEQ, <=, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, INT_GEQ, >=, as_bool, Kind::PT_BOOL, int32_t)
            _processOpCodeMath(this, int32_t, as_int, ADD_INT, +, as_int, Kind::PT_INT32, int32_t)
            _processOpCodeMath(this, int32_t, as_int, SUB_INT, -, as_int, Kind::PT_INT32, int32_t)
            _processOpCodeMath(this, int32_t, as_int, MUL_INT, *, as_int, Kind::PT_INT32, int32_t)
            _processOpCodeMath(this, int32_t, as_int, DIV_INT, /, as_float, Kind::PT_FLOAT32, _Float32)
            _processOpCodeMathUnary(this, int32_t, as_int, INT_NEG, -, as_int, Kind::PT_INT32, int32_t);

            _processOpCodeMath(this, _Float32, as_float, FLOAT_EQ, ==, as_bool, Kind::PT_BOOL, _Float32);
            _processOpCodeMath(this, _Float32, as_float, FLOAT_NEQ, !=, as_bool, Kind::PT_BOOL, _Float32);
            _processOpCodeMath(this, _Float32, as_float, FLOAT_LE, <, as_bool, Kind::PT_BOOL, _Float32)
            _processOpCodeMath(this, _Float32, as_float, FLOAT_GE, >, as_bool, Kind::PT_BOOL, _Float32)
            _processOpCodeMath(this, _Float32, as_float, FLOAT_LEQ, <=, as_bool, Kind::PT_BOOL, _Float32)
            _processOpCodeMath(this, _Float32, as_float, FLOAT_GEQ, >=, as_bool, Kind::PT_BOOL, _Float32)
            _processOpCodeMath(this, _Float32, as_float, ADD_FLOAT, +, as_float, Kind::PT_FLOAT32, _Float32)
            _processOpCodeMath(this, _Float32, as_float, SUB_FLOAT, -, as_float, Kind::PT_FLOAT32, _Float32)
            _processOpCodeMath(this, _Float32, as_float, MUL_FLOAT, *, as_float, Kind::PT_FLOAT32, _Float32)
            _processOpCodeMath(this, _Float32, as_float, DIV_FLOAT, /, as_float, Kind::PT_FLOAT32, _Float32)
            _processOpCodeMathUnary(this, _Float32, as_float, FLOAT_NEG, -, as_float, Kind::PT_FLOAT32, _Float32);

            _processOpCodeMath(this, bool, as_bool, BOOL_EQ, ==, as_bool, Kind::PT_BOOL, bool);
            _processOpCodeMath(this, bool, as_bool, BOOL_NEQ, !=, as_bool, Kind::PT_BOOL, bool);
            _processOpCodeMath(this, bool, as_bool, BOOL_AND, &&, as_bool, Kind::PT_BOOL, bool);
            _processOpCodeMath(this, bool, as_bool, BOOL_OR, || , as_bool, Kind::PT_BOOL, bool);
            _processOpCodeMathUnary(this, bool, as_bool, BOOL_NOT, !, as_bool, Kind::PT_BOOL, bool);
            DEFAULT : {
                fprintf(stderr, "VM operations handler error : Unknown operation code (%d)\n", CURRENT_CODE(this));
                exit(12);
            }
            CASE(HALT) : goto vm_end;
    #ifndef USELABELS
        } }
    #endif

        vm_end :

        return 0;
    }
}