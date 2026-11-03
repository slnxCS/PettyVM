#ifndef STACK_H
#define STACK_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

template<typename T>
class Stack {
    static_assert(std::is_trivially_copyable_v<T>, "Stack supports only trivially copyable types!");

    private : 
        size_t capacity;
        T* ptr;

        static constexpr size_t STACK_C_MAX = 10000000;
        static constexpr size_t STACK_C_START = 64;

        inline constexpr size_t STACK_GET_REALLOC_C(size_t old) {
            return (old) < STACK_C_MAX / 2 ? (old) * 2 : STACK_C_MAX;
        }

        inline void allocatePtr(size_t allocate_len) {
            ptr = (T*)malloc(allocate_len);
            if (ptr == NULL) 
            {
                std::fprintf(stderr, "Stack memory allocation error : cannot allocate %zu bytes\n", allocate_len);
                exit(1);
            }
        }

        inline void reallocatePtr(size_t reallocate_len) {
            auto temp = (T*)realloc(ptr, reallocate_len);
            if (temp == NULL) 
            {
                std::fprintf(stderr, "Stack memory reallocation error : cannot reallocate %zu bytes\n", reallocate_len);
                exit(1);
            }

            ptr = temp;
        }
    public : 
        size_t TopIndex;

        Stack(const Stack&) = delete;
        Stack& operator=(const Stack&) = delete;

        T& operator [](uint64_t index) {
            if (index >= TopIndex) 
            {
                fprintf(stderr, "Stack error : index was outside the bounds of the stack");
                exit(15);
            }

            return ptr[index];
        }

        Stack(size_t capacity = STACK_C_MAX) {
            this->capacity = capacity;
            TopIndex = 0;
            allocatePtr(this->capacity * sizeof(T));
        }

        Stack(Stack&& other) noexcept : capacity(other.capacity), TopIndex(other.TopIndex), ptr(other.ptr) {
            other.ptr = nullptr;
            other.TopIndex = 0;
            other.capacity = 0;
        }

        ~Stack() {
            free(ptr);
        }

        inline void Push(const T& value) {
            if (TopIndex >= capacity) {
                if (capacity >= STACK_C_MAX) {
                    fprintf(stderr, "StackOverflow error : reached maximum length(%zu)\n", STACK_C_MAX);
                    exit(1);
                }
                
                capacity = STACK_GET_REALLOC_C(capacity);
                reallocatePtr(capacity * sizeof(T));
            }

            ptr[TopIndex] = value;
            TopIndex++;
        }

        T Pop() {
            if (TopIndex <= 0) {
                fprintf(stderr, "Stack pop error : top index == 0\n");
                exit(1);
            }

            return ptr[--TopIndex];
        }

        inline T Peek() const {
            return ptr[TopIndex - 1];
        }

        const T* Ptr() const { return ptr; }
};

#endif // STACK_H