#ifndef BUILTINFUNCTIONS_HPP
#define BUILTINFUNCTIONS_HPP

#include <cstdio>
#include <cstring>
#include <cmath>
#include <random>

#include "./defs.hpp"

extern void _getchar();
extern void _getline();
extern void _out(string);
extern void _tmrs();
extern decimal _tmrr();
extern integer _rand();
extern void _hzero(integer mem, integer arrlength);
extern void _arrcopy(integer dst, void* src, integer datasize);

extern char strbuffer[];
extern instruction first_read;

extern pointer alloc(instruction size);

extern string alloc_string(const char* source);

extern string typecast_integer_string(integer);
extern string typecast_decimal_string(decimal);
extern string typecast_ascii_string(ascii);
extern integer typecast_string_integer(string);
extern decimal typecast_string_decimal(string);

extern string stradd(string, string);
extern integer intpow(integer, integer);
extern decimal decpow(decimal, decimal);

extern bool streq(string, string);
extern bool strneq(string, string);
extern bool strgte(string, string);
extern bool strgt(string, string);
extern bool strlte(string, string);
extern bool strlt(string, string);

#endif