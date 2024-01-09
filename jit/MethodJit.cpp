
#include "./MethodJit.hpp"

vec<method_jit::jit::hanging_jump> method_jit::jit::hanging_jumps;

vec<method_jit::jit::rewritable_function_call> method_jit::jit::rewritable_function_calls;
#include <iostream>
void build_next_function(sljit_sw cur_sp) {

    auto cur_stack = (instruction*)cur_sp;
    auto old_ip = cur_stack[1];

    auto new_ip = program[old_ip - 1] + 1;
    ip = new_ip;

    jit_function* jit_func;

    if (program[new_ip] == 0) {

        auto jit = method_jit::jit();

        jit_func = jit.func;
        program[new_ip] = (instruction)jit_func;

    }
    else jit_func = (jit_function*)program[new_ip];

    auto exec_offset = (sljit_sw)program[old_ip + 1];
    auto orig_ip = old_ip;

    while (true) {

        sljit_set_jump_addr((sljit_uw)program[old_ip], SLJIT_FUNC_UADDR(jit_func), exec_offset);

        new_ip = program[old_ip + 2];

        if (new_ip < 0) break;
        if (new_ip == orig_ip) break;
        
        old_ip = new_ip;

    }
    
    jit_func(cur_sp);

}

static void dump_code(void *code, sljit_uw len)
{
    FILE *fp = fopen("/tmp/slj_dump", "wb");
    if (!fp)
    return;
    fwrite(code, len, 1, fp);
    fclose(fp);
    #if defined(SLJIT_CONFIG_X86_64)
    system("objdump -b binary -m l1om -D /tmp/slj_dump");
    #elif defined(SLJIT_CONFIG_X86_32)
    system("objdump -b binary -m i386 -D /tmp/slj_dump");
    #endif
}
method_jit::jit::jit() {

    ip_temp = ip;

    hanging_jumps.clear();
    rewritable_function_calls.clear();

    //std::cout << "\nJITTING!\n";
    //std::cout << "IP is " << ip << "\n";
    //std::cout << "Number of local args is " << program[ip] << "\n";

    compiler = sljit_create_compiler(NULL, NULL);
    cur_scope = 0;

    sljit_emit_enter(compiler, 0, SLJIT_ARGS1V(W), SLJIT_NUMBER_OF_SCRATCH_REGISTERS, SLJIT_NUMBER_OF_SAVED_REGISTERS, SLJIT_NUMBER_OF_FLOAT_REGISTERS, 0, 0);

    while (prepare_jumps()); 
    //for (auto hj : hanging_jumps) std::cout << "FOUND HJ -- " << hj.ip_jump << " : " << hj.ip_target << "\n";
    while (next_instruction());

    for (auto& hj : hanging_jumps)
        sljit_set_label(hj.jump, hj.target);

    func = (jit_function*)sljit_generate_code(compiler);
    exec_offset = sljit_get_executable_offset(compiler);
    //auto shitlen = sljit_get_generated_code_size(compiler);dump_code((void*)func, shitlen);
    for (auto& rwfc : rewritable_function_calls) {

        auto call_addr = sljit_get_jump_addr(rwfc.jump);
        //std::cout << "Inserting call_addr and exec_offset at " << rwfc.ip_jaddr << "\n";
        program[rwfc.ip_jaddr] = (instruction)call_addr;
        program[rwfc.ip_jaddr + 1] = (instruction)exec_offset;
        program[rwfc.ip_jaddr + 2] = rwfc.ip_jaddr_next;
        //std::cout << "\tcall_addr=" << call_addr << "\n\texec_offset=" << exec_offset << "\n";
        sljit_set_jump_addr(call_addr, SLJIT_FUNC_UADDR(build_next_function), exec_offset);

    }

    sljit_free_compiler(compiler);
    //std::cout << "DONE JITTINH\n";
}

method_jit::jit::~jit() { if (func && false) sljit_free_code((void*)func, NULL); }

void method_jit::jit::next_addr(bool calc_mem_addr) {

    static const integer _temp_reg_1_[] = { TEMP_REGISTER_1_1, TEMP_REGISTER_1_2, TEMP_REGISTER_1_3 };
    static const integer _temp_reg_2_[] = { TEMP_REGISTER_2_1, TEMP_REGISTER_2_2, TEMP_REGISTER_2_3 };
    
    instruction mode = program[ip];

    bool is_decimal = mode / 16 == 1;

    auto& cur_addr = cur_inst.parts[cur_part_index];

    auto& addr_p1 = cur_addr.addr.args[0];
    auto& addr_p2 = cur_addr.addr.args[1];

    cur_addr.addr.addr_type = mode;

    mode %= 16;
    //std::cout << "\t\tProcessing addr: " << mode << " (#" << cur_part_index << ")" << " format=" << (program[ip]/16) << "\n";
    switch (mode) {

#define _temp_reg_1 _temp_reg_1_[cur_addr_index]
#define _temp_reg_2 _temp_reg_2_[cur_addr_index]

    case Eval::access::local:

        ip += 2;

        if (calc_mem_addr) {

            sljit_emit_op2(compiler, SLJIT_ADD, _temp_reg_1, 0, SP_REGISTER, 0, SLJIT_IMM, program[ip - 1] * 8);

            addr_p1 = _temp_reg_1;
            addr_p2 = 0;

        }

        else {

            addr_p1 = SLJIT_MEM1(SP_REGISTER);
            addr_p2 = program[ip - 1] * 8;

        }

        break;

    case Eval::access::global:

        ip += 2;

        addr_p1 = calc_mem_addr ? SLJIT_IMM : SLJIT_MEM;
        addr_p2 = (sljit_sw)(stack + program[ip - 1]);

        break;

    case Eval::access::imm:

        ip += 2;

        addr_p1 = is_decimal ? SLJIT_MEM : SLJIT_IMM;
        addr_p2 = is_decimal ? (sljit_sw)(program + (ip - 1)) : program[ip - 1];

        break;

#define _load_local_arg(reg, n) sljit_emit_op1(compiler, SLJIT_MOV, reg, 0, SLJIT_MEM1(SP_REGISTER), program[ip - n] * 8)
#define _load_global_arg(reg, n) sljit_emit_op1(compiler, SLJIT_MOV, reg, 0, SLJIT_MEM, (sljit_sw)(stack + program[ip - n]))

#define _load_first_local_arg() _load_local_arg(_temp_reg_1, 2); sljit_emit_op2(compiler, SLJIT_ADD, _temp_reg_1, 0, SLJIT_IMM, (sljit_sw)heap, _temp_reg_1, 0)
#define _load_first_global_arg() _load_global_arg(_temp_reg_1, 2); sljit_emit_op2(compiler, SLJIT_ADD, _temp_reg_1, 0, SLJIT_IMM, (sljit_sw)heap, _temp_reg_1, 0)
#define _load_second_local_arg() _load_local_arg(_temp_reg_2, 1)
#define _load_second_global_arg() _load_global_arg(_temp_reg_2, 1)
#define _load_second_imm_arg() sljit_emit_op1(compiler, SLJIT_MOV, _temp_reg_2, 0, is_decimal ? SLJIT_MEM : SLJIT_IMM, is_decimal ? (sljit_sw)(program + (ip - 1)) : program[ip - 1])

#define _heap_access_case(type, load1, load2, shift) \
\
case Eval::access::type: \
    \
    ip += 3; \
    \
    load1; \
    load2; \
    \
    if (calc_mem_addr) { \
        \
        if (shift == 3) sljit_emit_op2(compiler, SLJIT_MUL, _temp_reg_2, 0, _temp_reg_2, 0, SLJIT_IMM, 8); \
        sljit_emit_op2(compiler, SLJIT_ADD, _temp_reg_1, 0, _temp_reg_1, 0, _temp_reg_2, 0); \
        \
        addr_p1 = _temp_reg_1; \
        addr_p2 = 0; \
        \
    } \
    \
    else { \
        \
        addr_p1 = SLJIT_MEM2(_temp_reg_1, _temp_reg_2); \
        addr_p2 = shift; \
        \
    } \
    \
    break

#define _heap_cases(numbytes, shift) \
\
_heap_access_case(heap##numbytes##ll, _load_first_local_arg(), _load_second_local_arg(), shift); \
_heap_access_case(heap##numbytes##gl, _load_first_global_arg(), _load_second_local_arg(), shift); \
_heap_access_case(heap##numbytes##lg, _load_first_local_arg(), _load_second_global_arg(), shift); \
_heap_access_case(heap##numbytes##gg, _load_first_global_arg(), _load_second_global_arg(), shift); \
_heap_access_case(heap##numbytes##li, _load_first_local_arg(), _load_second_imm_arg(), shift); \
_heap_access_case(heap##numbytes##gi, _load_first_global_arg(), _load_second_imm_arg(), shift)

    _heap_cases(1, 0);
    _heap_cases(8, 3);

    }

    cur_part_index++;
    cur_addr_index++;

}