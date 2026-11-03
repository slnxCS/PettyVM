#include "file_reader.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <fstream>

uint8_t* Petty::read_file(const char* fname) 
{
    std::ifstream file(fname, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::fprintf(stderr, "Error : cannot open file '%s'\n", fname);
        exit(2);
    }

    size_t len = file.tellg();

    file.seekg(0, std::ios::beg);

    uint8_t* buf = new uint8_t[len];

    if (!buf) {
        std::fprintf(stderr, "Error : cannot allocate %zu bytes\n", len);
        exit(1);        
    }

    if (!file.read(reinterpret_cast<char*>(buf), len)) {
        delete[] buf;
        std::fprintf(stderr, "Error : cannot read file '%s'", fname);
        exit(2);
    }

    return buf;
}