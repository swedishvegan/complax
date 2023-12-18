#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <random>
#include "./util/Timer.hpp"
#include "./Eval/Instruction.hpp"

#define stack_size (1<<23)
#define initial_heap_size (1<<23)

using namespace Eval;

using pointer = instruction;

using integer = int64_t;
using decimal = double;
using ascii = unsigned char;
using string = instruction;

char* heap = nullptr;
pointer hp = 0;

instruction* _stack = nullptr;
pointer _sp = 0;

instruction* _program = nullptr;
pointer _ip = 0;

bool init(std::string filename) {

    try {

        std::ifstream input_file(filename, std::ios::binary | std::ios::in);

        if (!input_file.is_open()) { std::cout << "Failed to open file '" << filename << "'.\n"; return false; }

        char signature[15] = { };

        input_file.read(signature, 14);
        signature[14] = (char)0;

        if (std::string(signature) != "LAX-EXECUTABLE") {

            std::cout << "File '" << filename << "' is not a Lax executable.\n";
            return false;

        }

        size_t program_data_size = 0;
        size_t stack_data_size = 0;
        size_t instructions_size = 0;

        input_file >> program_data_size;

        if (program_data_size > initial_heap_size) {

            std::cout << "Program data for '" << filename << "' is too large.\n";
            return false;

        }

        heap = new char[initial_heap_size];
        hp = program_data_size;

        input_file.read(heap, program_data_size);

        input_file >> stack_data_size;

        if (stack_data_size > stack_size) {

            std::cout << "Stack data for '" << filename << "' is too large.\n";
            return false;

        }

        _stack = new instruction[stack_size];
        _sp = stack_data_size;

        input_file.read((char*)_stack, stack_data_size * sizeof(instruction));
        input_file >> instructions_size;

        _program = new instruction[instructions_size];
        input_file.read((char*)_program, instructions_size * sizeof(instruction));
        
    }
    catch(...) {

        std::cout << "Failed to open file '" + filename + "'.\n";
        return false;

    }

    return true;

}

char strbuffer[2048];   // Used for cstring operations like scanf
bool first_read = true; // getchar() if not first read

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

// Below are some typecast implementations used for various "cast" instructions

#define typecast_impl(name, primitive, format)                              \
                                                                            \
inline string typecast_##name##_string(primitive v) {                       \
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

typecast_impl(integer, long, "%ld")
typecast_impl(decimal, decimal, "%f")

const char* _true = "true";
const char* _false = "false";

inline string typecast_bool_string(bool v) {

    pointer len = v ? 4 : 5;
    string p_str = alloc(len + 1);

    char* str = heap + p_str;
    for (pointer i = 0; i < len; i++) str[i] = v ? _true[i] : _false[i];
    str[len] = (char)0;

    return p_str;

}

inline string typecast_ascii_string(ascii a) {

    string p_str = alloc(2);

    char* str = heap + p_str;
    str[0] = (char)a;
    str[1] = (char)0;

    return p_str;

}

inline decimal typecast_string_decimal(string s) { return (decimal)atoll(heap + s); }

inline integer typecast_string_integer(string s) { return (integer)atof(heap + s); }

inline bool typecast_string_bool(string s) { return strcmp(heap + s, _true) == 0; }

inline string stradd(string p_s1, string p_s2) {

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

inline integer intpow(integer base, integer exp) {

	integer result = 1;
	while (exp > 0) { if (exp % 2) result *= base; exp /= 2; base *= base; }

	return result;

}

#define strcomp_impl(op_name, comp)                            \
                                                               \
inline bool str##op_name(string p_s1, string p_s2) {           \
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

int main(int argc, char** argv) {

    if (argc < 2) { std::cout << "Invalid number of arguments.\n"; return 1; }

    srand(clock());

    pointer ip = 0, sp = 0;

    instruction* stack = nullptr;
    instruction* program = nullptr;

#define stack_element(i, T) *(T*)(stack + i)    
#define local_stack_element(i, T) *(T*)(stack + sp + i)
#define arg(i, T) *(T*)(program + ip + i)
#define stack_arg(i, T) stack_element(arg(i, instruction), T)
#define local_stack_arg(i, T) local_stack_element(arg(i, instruction), T)
#define finish(code) if (heap) delete[] heap; if (_stack) delete[] _stack; if (_program) delete[] _program; return code
#define heap_arg(base, index, T) (*(T*)(heap + base + index))

#define nothing_accessed() { std::cout << "\nAttempt to access nothing.\n"; return 1; }

    if (!init(argv[1])) { finish(1); }

    stack = _stack;
    program = _program;

    sp = _sp;

    Timer timer;

    while (true) switch (program[ip]) {

    case inst::exit:

        finish(0);

    case inst::sass:

        if (sp + arg(1, instruction) >= stack_size) {

            printf("\nStack overflow.\n");
            finish(1);

        }

        ip += 2;
        break;

    case inst::j:

        ip = arg(1, instruction);
        break;

    case inst::jc:

        ip = local_stack_arg(2, bool) ? arg(1, instruction) : ip + 3;
        break;

    case inst::call:

    {

        auto sp_restore = sp;
        auto ip_restore = ip + 3;

        sp += arg(1, instruction);
        ip = arg(2, instruction);

        local_stack_element(0, instruction) = sp_restore;
        local_stack_element(1, instruction) = ip_restore;

    }

        break;

    case inst::ret:

        ip = local_stack_element(1, instruction);
        sp = local_stack_element(0, instruction);

        break;

    case inst::inp:

    {

        if (first_read) first_read = false;
        else auto _dummy = getchar();

        auto _dummy = scanf("%2047[^\n]", strbuffer);
        local_stack_arg(1, string) = alloc_string(strbuffer);

    }

        ip += 2;
        break;

    case inst::out:

        printf("%s", heap + local_stack_arg(1, string));

        ip += 2;
        break;

    case inst::tmrs:

        timer.reset();

        ip++;
        break;

    case inst::tmrr:

        local_stack_arg(1, decimal) = timer.time();
    
        ip += 2;
        break;

    case inst::rand:

        local_stack_arg(1, integer) = (integer)std::rand();

        ip += 2;
        break;

    case inst::gc:

        /* Garbage collection here */
        break;

    case inst::halloc:

        local_stack_arg(2, instruction) = alloc(local_stack_arg(1, instruction) * 8);

        ip += 3;
        break;

    case inst::hinit:

    {

        auto arrlength = local_stack_arg(1, instruction) * 8;
        auto allocation = alloc(arrlength);

        std::memset(heap + allocation, 0, 8 * arrlength);

        local_stack_arg(2, instruction) = allocation;

    }

        ip += 3;
        break;

    case inst::lhcpy:

    {

        auto heapoffset = local_stack_arg(3, instruction);
        if (heapoffset == 0) nothing_accessed();

        std::memcpy(heap + heapoffset, stack + sp + arg(2, instruction), 8 * local_stack_arg(1, instruction));

    }

        ip += 4;
        break;

    case inst::hhcpy:

    {

        auto heapoffsetdst  = local_stack_arg(3, instruction);
        if (heapoffsetdst == 0) nothing_accessed();

        auto heapoffsetsrc  = local_stack_arg(2, instruction);
        if (heapoffsetsrc == 0) nothing_accessed();

        std::memcpy(heap + heapoffsetdst, heap + heapoffsetsrc, 8 * local_stack_arg(1, instruction));

    }

        ip += 4;
        break;

    case inst::movll:

        local_stack_arg(2, instruction) = local_stack_arg(1, instruction);

        ip += 3;
        break;

    case inst::movpl:

        local_stack_arg(2, instruction) = arg(1, instruction);

        ip += 3;
        break;

    case inst::movxl:

        local_stack_arg(2, instruction) = stack_arg(1, instruction);

        ip += 3;
        break;

    case inst::movhl:

    {

        auto heapoffset = local_stack_arg(2, instruction);
        if (heapoffset == 0) nothing_accessed();

        local_stack_arg(3, instruction) = heap_arg(heapoffset, 8 * local_stack_arg(1, instruction), instruction);

    }

        ip += 4;
        break;

    case inst::movlx:

        stack_arg(2, instruction) = local_stack_arg(1, instruction);

        ip += 3;
        break;

    case inst::movpx:

        stack_arg(2, instruction) = arg(1, instruction);

        ip += 3;
        break;

    case inst::movxx:

        stack_arg(2, instruction) = stack_arg(1, instruction);

        ip += 3;
        break;

    case inst::movhx:

    {

        auto heapoffset = local_stack_arg(2, instruction);
        if (heapoffset == 0) nothing_accessed();

        stack_arg(3, instruction) = heap_arg(heapoffset, 8 * local_stack_arg(1, instruction), instruction);

    }

        ip += 4;
        break;

    case inst::movlh:

    {

        auto heapoffset = local_stack_arg(3, instruction);
        if (heapoffset == 0) nothing_accessed();

        heap_arg(heapoffset, 8 * local_stack_arg(2, instruction), instruction) = local_stack_arg(1, instruction);

    }

        ip += 4;
        break;

    case inst::movph:

    {

        auto heapoffset = local_stack_arg(3, instruction);
        if (heapoffset == 0) nothing_accessed();

        heap_arg(heapoffset, 8 * local_stack_arg(2, instruction), instruction) = arg(1, instruction);

    }

        ip += 4;
        break;

    case inst::movxh:

    {

        auto heapoffset = local_stack_arg(3, instruction);
        if (heapoffset == 0) nothing_accessed();

        heap_arg(heapoffset, 8 * local_stack_arg(2, instruction), instruction) = stack_arg(1, instruction);

    }

        ip += 4;
        break;

    case inst::movhh:

    {

        auto heapoffsetdst = local_stack_arg(4, instruction);
        if (heapoffsetdst == 0) nothing_accessed();

        auto heapoffsetsrc = local_stack_arg(2, instruction);
        if (heapoffsetsrc == 0) nothing_accessed();

        heap_arg(heapoffsetdst, 8 * local_stack_arg(3, instruction), instruction) = heap_arg(heapoffsetsrc, 8 * local_stack_arg(1, instruction), instruction);

    }

        ip += 5;
        break;

    case inst::strlen:

        local_stack_arg(2, instruction) = *((instruction*)(heap + local_stack_arg(1, instruction)) - 1) - 1;

        ip += 3;
        break;

    case inst::arrlen:

    {

        auto heapoffset = local_stack_arg(1, instruction);
        if (heapoffset == 0) nothing_accessed();

        local_stack_arg(2, instruction) = *((instruction*)(heap + heapoffset) - 1) / 8;

    }

        ip += 3;
        break;

    case inst::arrcat:

    {

        auto heapoffset1 = local_stack_arg(1, instruction);
        if (heapoffset1 == 0) nothing_accessed();

        auto heapoffset2 = local_stack_arg(2, instruction);
        if (heapoffset2 == 0) nothing_accessed();

        auto arr1 = (instruction*)(heap + heapoffset1);
        auto arr2 = (instruction*)(heap + heapoffset2);

        auto sz1 = *(arr1 - 1);
        auto sz2 = *(arr2 - 1);

        auto catresult = alloc(sz1 + sz2);
        auto arrres = (instruction*)(heap + catresult);

        std::memcpy(arrres, arr1, sz1);
        std::memcpy(arrres + sz1/8, arr2, sz2);

        local_stack_arg(3, instruction) = catresult;

    }

        ip += 4;
        break;

#define cast_instruction(ltype, rtype, lreg, rreg) case inst::cast##lreg##rreg: local_stack_arg(2, rtype) = (rtype)local_stack_arg(1, ltype); ip += 3; break

    cast_instruction(integer, decimal, i, d);
    cast_instruction(decimal, integer, d, i);
    cast_instruction(integer, bool, i, b);
    cast_instruction(bool, integer, b, i);
    cast_instruction(decimal, bool, d, b);
    cast_instruction(bool, decimal, b, d);
    cast_instruction(ascii, integer, a, i);
    cast_instruction(integer, ascii, i, a);

#define str_cast_instruction(ltype, rtype, lreg, rreg) case inst::cast##lreg##rreg: local_stack_arg(2, rtype) = typecast_##ltype##_##rtype(local_stack_arg(1, ltype)); ip += 3; break

    str_cast_instruction(string, integer, s, i);
    str_cast_instruction(integer, string, i, s);
    str_cast_instruction(string, decimal, s, d);
    str_cast_instruction(decimal, string, d, s);
    str_cast_instruction(string, bool, s, b);
    str_cast_instruction(bool, string, b, s);
    str_cast_instruction(ascii, string, a, s);

#define op_instruction(op, opname, dt, dtype) \
                                              \
case inst::opname##dt:                        \
                                              \
local_stack_arg(3, dtype) =                   \
    local_stack_arg(1, dtype)                 \
        op                                    \
    local_stack_arg(2, dtype);                \
                                              \
ip += 4;                                      \
break

    op_instruction(+, add, i, integer);
    op_instruction(+, add, d, decimal);
    op_instruction(+, add, a, ascii);
    op_instruction(-, sub, i, integer);
    op_instruction(-, sub, d, decimal);
    op_instruction(-, sub, a, ascii);
    op_instruction(*, mul, i, integer);
    op_instruction(*, mul, d, decimal);
    op_instruction(/, div, i, integer);
    op_instruction(/, div, d, decimal);
    op_instruction(&, and, i, integer);
    op_instruction(&, and, b, bool);
    op_instruction(|, or, i, integer);
    op_instruction(|, or, b, bool);
    op_instruction(%, mod, i, integer);

#define op_instruction_special(labl, dtype, funcname) \
                                                      \
case inst::labl:                                      \
                                                      \
local_stack_arg(3, dtype) = funcname(                 \
    local_stack_arg(1, dtype),                        \
    local_stack_arg(2, dtype)                         \
);                                                    \
                                                      \
ip += 4;                                              \
break;

    op_instruction_special(powi, integer, intpow);
    op_instruction_special(powd, decimal, powf);
    op_instruction_special(adds, string, stradd);

#define comp_instruction(comp, compname, dt, dtype) \
                                                    \
case inst::compname##dt:                            \
                                                    \
local_stack_arg(3, bool) =                          \
    local_stack_arg(1, dtype)                       \
        comp                                        \
    local_stack_arg(2, dtype);                      \
                                                    \
ip += 4;                                            \
break

    comp_instruction(==, eq, i, integer);
    comp_instruction(==, eq, d, decimal);
    comp_instruction(==, eq, b, bool);
    comp_instruction(==, eq, a, ascii);
    comp_instruction(!=, neq, i, integer);
    comp_instruction(!=, neq, d, decimal);
    comp_instruction(!=, neq, b, bool);
    comp_instruction(!= , neq, a, ascii);
    comp_instruction(>, gt, i, integer);
    comp_instruction(>, gt, d, decimal);
    comp_instruction(>, gt, a, ascii);
    comp_instruction(>=, gte, i, integer);
    comp_instruction(>=, gte, d, decimal);
    comp_instruction(>=, gte, a, ascii);
    comp_instruction(<, lt, i, integer);
    comp_instruction(<, lt, d, decimal);
    comp_instruction(<, lt, a, ascii);
    comp_instruction(<=, lte, i, integer);
    comp_instruction(<=, lte, d, decimal);
    comp_instruction(<=, lte, a, ascii);

#define strcomp_instruction(compname)       \
                                            \
case inst::compname##s:                     \
                                            \
local_stack_arg(3, string) = str##compname( \
    local_stack_arg(1, string),             \
    local_stack_arg(2, string)              \
);                                          \
                                            \
ip += 4;                                    \
break

    strcomp_instruction(eq);
    strcomp_instruction(neq);
    strcomp_instruction(gt);
    strcomp_instruction(gte);
    strcomp_instruction(lt);
    strcomp_instruction(lte);

    case inst::noti: local_stack_arg(2, integer) = !(local_stack_arg(1, integer)); ip += 3; break;

    case inst::notb: local_stack_arg(2, bool) = !(local_stack_arg(1, bool)); ip += 3; break;

    }

}