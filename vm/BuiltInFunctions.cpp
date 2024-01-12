
#include "./BuiltInFunctions.hpp"

void _getchar() { auto dummy = getchar(); }

void _getline() { auto dummy = scanf("%2047[^\n]", strbuffer); }

void _out(string str) { printf("%s", heap + str); }

void _tmrs() { timer.reset(); }

decimal _tmrr() { return timer.time(); }

integer _rand() { return (integer)std::rand(); }

void _hzero(integer mem, integer arrlength) { std::memset(heap + mem, 0, arrlength); }

void _arrcopy(integer dst, void* src, integer datasize) { std::memcpy(heap + dst, src, datasize); }

char strbuffer[2048];
instruction first_read = true;

pointer alloc(instruction size) {

    hp += (8 - hp%8) % 8;
    *(instruction*)(heap + hp) = size;

    hp += size + 8;
    return hp - size;

}

string alloc_string(const char* source) {

    pointer size = 0;
    while (source[size]) size++;

    pointer p_str = alloc(size + 1);
    char* str = heap + p_str;

    for (pointer i = 0; i < size; i++) str[i] = source[i];
    str[size] = (char)0;
    
    return p_str; 

}

#define typecast_impl(name, primitive, format)                              \
                                                                            \
string typecast_##name##_string(primitive v) {                              \
                                                                            \
    pointer len = (pointer)snprintf(strbuffer, 256, format, v);             \
    string p_str = alloc(len + 1);                                          \
                                                                            \
    char* str = heap + p_str;                                               \
    for (pointer i = 0; i < len; i++) str[i] = strbuffer[i];                \
    str[len] = (char)0;                                                     \
                                                                            \
    return p_str;                                                           \
                                                                            \
}

typecast_impl(integer, integer, "%ld")
typecast_impl(decimal, decimal, "%f")

string typecast_ascii_string(ascii a) {

    string p_str = alloc(2);

    char* str = heap + p_str;
    str[0] = (char)a;
    str[1] = (char)0;

    return p_str;

}

integer typecast_string_integer(string s) { return (integer)atoll(heap + s); }

decimal typecast_string_decimal(string s) { return (decimal)atof(heap + s); }

string stradd(string p_s1, string p_s2) {

    const char* s1 = heap + p_s1;
    const char* s2 = heap + p_s2;

    pointer size1 = 0;
    while (s1[size1]) size1++;

    pointer size2 = 0;
    while (s2[size2]) size2++;

    auto size = size1 + size2;
    string p_str = alloc(size + 1);
    auto str = heap + p_str;

    for (pointer i = 0; i < size1; i++) str[i] = s1[i];
    for (pointer i = 0; i < size2; i++) str[size1 + i] = s2[i];

    str[size] = (char)0;
    return p_str;

}

integer intpow(integer base, integer exp) {

	integer result = 1;
	while (exp > 0) { if (exp % 2) result *= base; exp /= 2; base *= base; }

	return result;

}

#define strcomp_impl(op_name, comp)                            \
                                                               \
bool str##op_name(string p_s1, string p_s2) {                  \
                                                               \
    const char* s1 = heap + p_s1;                              \
    const char* s2 = heap + p_s2;                              \
                                                               \
    return strcmp(s1, s2) comp 0;                              \
                                                               \
}

strcomp_impl(eq, ==)
strcomp_impl(neq, !=)
strcomp_impl(gte, >=)
strcomp_impl(gt, >)
strcomp_impl(lte, <=)
strcomp_impl(lt, <)