
#include "./defs.hpp"

char* heap = nullptr;
pointer hp = 0;

instruction* stack = nullptr;
pointer sp = 0;

instruction* program = nullptr;
pointer ip = 1;

Timer timer;