#ifndef CHECKER_H
#define CHECKER_H

#include <cstdlib>
#include <cstdio>

namespace Petty {
    class Checker {
        private :
            [[noreturn]] static inline void ThrowNullRefException() {
                fprintf(stderr, "Fatal error : null reference exception\n");
                exit(1);
            }

            [[noreturn]] static inline void ThrowCannotAllocException(const size_t& alloc_len) {
                fprintf(stderr, "Fatal error : cannot allocate %zu bytes\n", alloc_len);
                exit(1);
            }
        public :
            template<typename T>
            static inline void CheckNotNull(const T* ptr) {
                if (ptr == nullptr) ThrowNullRefException();
            }

            template<typename T>
            static inline void CheckNotNullAllocated(const T* ptr, const size_t& allocate_len) {
                if (ptr == nullptr) ThrowCannotAllocException(allocate_len);
            }
    };
}

#endif // CHECKER_H