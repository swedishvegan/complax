#ifndef VM_DEFS_HPP
#define VM_DEFS_HPP

#include <cstdint>
#include "./../util/Timer.hpp"

#define stack_size (1<<23)
#define initial_heap_size (1<<23)
#define jit_lookahead_distance 3

using instruction = int64_t;
using pointer = instruction;

using integer = int64_t;
using decimal = double;
using ascii = unsigned char;
using string = instruction;

using jit_function = void(integer);

extern char* heap;
extern pointer hp;

extern instruction* stack;
extern pointer sp;

extern instruction* program;
extern pointer ip;

extern Timer timer;

#define end_case() ip++; break

#endif