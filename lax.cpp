
#include <iostream>
#include <fstream>
#include "./vm/BuiltInFunctions.hpp"
#include "./jit/MethodJit.hpp"

using namespace Eval;

bool use_jit = false;
bool argparse_fail = false;

std::string argparse(int argc, char** argv) {

    if (argc < 2) return "";

    bool jit = false;
    bool interpret = false;

    for (int i = 1; i < argc - 1; i++) {

        std::string flag = argv[i];

        if (flag == "--jit") jit = true;
        else if (flag == "--interpret") interpret = true;

        else {

            std::cout << "Unrecognized flag: '" << flag << "'. Argument format: [optional flags (--jit or --interpret)] [input filename]\n";

            argparse_fail = true;
            return "";

        }

    }

    if (jit && interpret) {

        std::cout << "Flags --jit and --interpret cannot both be specified.\n";

        argparse_fail = true;
        return "";

    }

    use_jit = jit;

    return argv[argc - 1];

}

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

        stack = new instruction[stack_size];
        sp = stack_data_size;

        input_file.read((char*)stack, stack_data_size * sizeof(instruction));
        input_file >> instructions_size;

        program = new instruction[instructions_size];
        input_file.read((char*)program, instructions_size * sizeof(instruction));
        
    }
    catch(...) {

        std::cout << "Failed to open file '" + filename + "'.\n";
        return false;

    }

    return true;

}

bool single_byte = false;

inline char* next_arg() {

    ip++;
    return (char*)(program + (ip - 1));

}

#define arg(T) *(T*)next_arg()

inline char* next_addr() {

    instruction mode = program[ip] % 16;
    single_byte = false;

    switch (mode) {

    case access::local:

        ip += 2;
        return (char*)(stack + sp + program[ip - 1]);

    case access::global:

        ip += 2;
        return (char*)(stack + program[ip - 1]);

    case access::imm:

        ip += 2;
        return (char*)(program + (ip - 1));

#define local_arg(n) *(stack + sp + program[ip + n - 3])
#define global_arg(n) *(stack + program[ip + n - 3])

    case access::heap1ll:

        ip += 3; single_byte = true;
        return (char*)(heap + local_arg(1) + local_arg(2));

    case access::heap1gl:

        ip += 3; single_byte = true;
        return (char*)(heap + global_arg(1) + local_arg(2));

    case access::heap1lg:

        ip += 3; single_byte = true;
        return (char*)(heap + local_arg(1) + global_arg(2));

    case access::heap1gg:

        ip += 3; single_byte = true;
        return (char*)(heap + global_arg(1) + global_arg(2));

    case access::heap1li:

        ip += 3; single_byte = true;
        return (char*)(heap + local_arg(1) + program[ip - 1]);

    case access::heap1gi:

        ip += 3; single_byte = true;
        return (char*)(heap + global_arg(1) + program[ip - 1]);


    case access::heap8ll:

        ip += 3;
        return (char*)(heap + local_arg(1) + 8 * local_arg(2));

    case access::heap8gl:

        ip += 3;
        return (char*)(heap + global_arg(1) + 8 * local_arg(2));

    case access::heap8lg:

        ip += 3;
        return (char*)(heap + local_arg(1) + 8 * global_arg(2));

    case access::heap8gg:

        ip += 3;
        return (char*)(heap + global_arg(1) + 8 * global_arg(2));

    case access::heap8li:

        ip += 3;
        return (char*)(heap + local_arg(1) + 8 * program[ip - 1]);

    case access::heap8gi:

        ip += 3;
        return (char*)(heap + global_arg(1) + 8 * program[ip - 1]);

    default:

        return nullptr;

    }

}

#define addr(T) *(T*)next_addr()

#define localStackElement(T, idx) (*(T*)(stack + sp + idx))

int main(int argc, char** argv) {

    srand(clock());

#define finish(code) if (heap) delete[] heap; if (stack) delete[] stack; if (program) delete[] program; return code
#define nothing_accessed() { std::cout << "\nAttempt to access nothing.\n"; finish(1); }

    auto filename = argparse(argc, argv);
    
    if (argparse_fail) { finish(1); }
    if (!init(filename)) { finish(1); }

    timer.reset();
    
    while (true) switch (program[ip - 1]) {
    
    case inst::exit:

        finish(0);

    case inst::begfnc:

        arg(instruction);
        end_case();

    case inst::j:

        ip = arg(instruction);
        end_case();

    case inst::jc:

        {

            auto jumptarget = arg(instruction);
            auto condition = !addr(bool);

            ip = condition ? jumptarget : ip;

        }
        
        end_case();

    case inst::call:

    {
        
        auto sp_restore = sp;
        auto ip_restore = ip + 5;

        sp += arg(instruction);
        ip = arg(instruction);

        localStackElement(instruction, 0) = sp_restore;
        localStackElement(instruction, 1) = ip_restore;

    }

        end_case();

    case inst::ret:

        ip = localStackElement(instruction, 1);
        sp = localStackElement(instruction, 0);

        end_case();

    case inst::scpbeg:

    if (use_jit) {

        auto jit = method_jit::jit();
        jit.func((integer)(stack + sp));

        finish(0);

    }

        arg(instruction);
        end_case();

    case inst::scpend:

        end_case();

    case inst::inp:

    {

        if (first_read) first_read = false;
        else auto _dummy = getchar();

        auto _dummy = scanf("%2047[^\n]", strbuffer);
        addr(string) = alloc_string(strbuffer);

    }

        end_case();

    case inst::out:

        printf("%s", heap + addr(string));
        end_case();

    case inst::tmrs:

        timer.reset();
        end_case();

    case inst::tmrr:

        addr(decimal) = timer.time();    
        end_case();

    case inst::rand:

        addr(integer) = (integer)std::rand();
        end_case();

    //case inst::gc:

        /* Garbage collection here */
        //end_case();

    case inst::halloc:

    {

        auto mem = alloc(addr(instruction));
        addr(instruction) = mem;

    }

        end_case();

    case inst::hinit:

    {

        auto arrlength = addr(instruction) * 8;
        auto mem = alloc(arrlength);

        std::memset(heap + mem, 0, arrlength);

        addr(instruction) = mem;

    }

        end_case();

    case inst::copy:

    {

        auto datasize = arg(instruction);
        auto& src = addr(instruction);
        auto dst = heap + addr(instruction);

        //if (src == 0 || (dst == 0 && )) nothing_accessed();

        std::memcpy(dst, &src, datasize);

    }

    end_case();

    case inst::mov:

    {

        auto is_single_byte = false;

        auto src = next_addr(); is_single_byte |= single_byte;
        auto dst = next_addr(); is_single_byte |= single_byte;

        if (is_single_byte) *(ascii*)dst = *(ascii*)src;
        else *(instruction*)dst = *(instruction*)src;

    }

    end_case();

    case inst::strlen:

    {

        auto straddress = addr(instruction);
        auto strlen = *((instruction*)(heap + straddress) - 1) - 1;

        addr(instruction) = strlen;

    }

        end_case();

    case inst::arrlen:

    {

        auto arraddress = addr(instruction);
        if (arraddress == 0) nothing_accessed();

        addr(instruction) = *((instruction*)(heap + arraddress) - 1) / 8;

    }

        end_case();

    case inst::arrcat:

    {

        auto heapoffset1 = addr(instruction);
        auto heapoffset2 = addr(instruction);

        if (heapoffset1 == 0 || heapoffset2 == 0) nothing_accessed();

        auto arr1 = (instruction*)(heap + heapoffset1);
        auto arr2 = (instruction*)(heap + heapoffset2);

        auto sz1 = *(arr1 - 1);
        auto sz2 = *(arr2 - 1);

        auto catresult = alloc(sz1 + sz2);
        auto arrres = (instruction*)(heap + catresult);

        std::memcpy(arrres, arr1, sz1);
        std::memcpy(arrres + sz1/8, arr2, sz2);

        addr(instruction) = catresult;

    }

        end_case();

#define cast_instruction(ltype, rtype, lreg, rreg) \
                                                   \
case inst::cast##lreg##rreg:                       \
                                                   \
{                                                  \
                                                   \
    auto val = (rtype)addr(ltype);                 \
    addr(rtype) = val;                             \
                                                   \
}                                                  \
                                                   \
end_case()

    cast_instruction(integer, decimal, i, d);
    cast_instruction(decimal, integer, d, i);
    cast_instruction(ascii, integer, a, i);
    cast_instruction(integer, ascii, i, a);

#define str_cast_instruction(ltype, rtype, lreg, rreg) \
                                                       \
case inst::cast##lreg##rreg:                           \
                                                       \
{                                                      \
                                                       \
    auto val = addr(ltype);                            \
    addr(rtype) = typecast_##ltype##_##rtype(val);     \
                                                       \
}                                                      \
                                                       \
end_case()

    str_cast_instruction(string, integer, s, i);
    str_cast_instruction(integer, string, i, s);
    str_cast_instruction(string, decimal, s, d);
    str_cast_instruction(decimal, string, d, s);
    str_cast_instruction(ascii, string, a, s);

#define op_instruction(op, opname, dt, dtype) \
                                              \
case inst::opname##dt:                        \
                                              \
{                                             \
                                              \
    auto val =                                \
        addr(dtype)                           \
            op                                \
        addr(dtype);                          \
                                              \
    addr(dtype) = val;                        \
                                              \
}                                             \
                                              \
end_case()

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
{                                                     \
                                                      \
    auto lh = addr(dtype);                            \
    auto rh = addr(dtype);                            \
                                                      \
    auto val = funcname(lh, rh);                      \
                                                      \
    addr(dtype) = val;                                \
                                                      \
}                                                     \
                                                      \
end_case()

    op_instruction_special(powi, integer, intpow);
    op_instruction_special(powd, decimal, powf);
    op_instruction_special(adds, string, stradd);

#define comp_instruction(comp, compname, dt, dtype) \
                                                    \
case inst::compname##dt:                            \
                                                    \
{                                                   \
                                                    \
    bool val =                                      \
        addr(dtype)                                 \
            comp                                    \
        addr(dtype);                                \
                                                    \
    addr(bool) = val;                               \
                                                    \
}                                                   \
                                                    \
end_case()

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

#define strcomp_instruction(compname) \
                                      \
case inst::compname##s:               \
                                      \
{                                     \
                                      \
    auto lh = addr(string);           \
    auto rh = addr(string);           \
                                      \
    bool val = str##compname(lh, rh); \
                                      \
    addr(bool) = val;                 \
                                      \
}                                     \
                                      \
end_case()

    strcomp_instruction(eq);
    strcomp_instruction(neq);
    strcomp_instruction(gt);
    strcomp_instruction(gte);
    strcomp_instruction(lt);
    strcomp_instruction(lte);

    case inst::noti:

    {

        auto negation = !addr(integer);
        addr(integer) = negation;

    }

        end_case();

    case inst::notb:

    {

        auto negation = !addr(bool);
        addr(bool) = negation;

    }

        end_case();
    
    }

}