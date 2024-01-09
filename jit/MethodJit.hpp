
#ifndef METHODJIT_HPP
#define METHODJIT_HPP

#include "./deps/sljit_src/sljitLir.h"
#include "./../vm/defs.hpp"
#include "./../vm/BuiltInFunctions.hpp"
#include "./../Eval/Instruction.hpp"
#include "./../util/vec.hpp"

#define SP_REGISTER SLJIT_S0

#define TEMP_REGISTER_1_1 SLJIT_R1
#define TEMP_REGISTER_1_2 SLJIT_R2
#define TEMP_REGISTER_1_3 SLJIT_R2

#define TEMP_REGISTER_2_1 SLJIT_R3
#define TEMP_REGISTER_2_2 SLJIT_R4
#define TEMP_REGISTER_2_3 SLJIT_R4

namespace method_jit {

    union inst_arg { integer iarg; decimal darg; ascii barg; };

    struct inst_addr {

#define inst_addr_format_decimal 1
#define inst_addr_format_integer 0
#define inst_addr_format_byte 2

        instruction addr_type; // First four bits store the Eval::access type, next eight bits store the format
        instruction args[2];

        inline instruction getFormat() { return addr_type / 16; }

    };

    inline bool operator == (const inst_addr& lh, const inst_addr& rh) {

        if (lh.addr_type != rh.addr_type) return false;
        if (lh.args[0] != rh.args[0]) return false;
        if (lh.addr_type <= Eval::access::imm) return true;
        return lh.args[1] == rh.args[1];

    }

    inline bool operator != (const inst_addr& lh, const inst_addr& rh) { return !(lh == rh); }

    struct inst_info {

        instruction opcode;
        union part { inst_arg arg; inst_addr addr; } parts[3];

    };

    struct jit { // JIT-compiles a function starting at the current IP value

        jit();

        ~jit();

        jit_function* func = nullptr;

    protected:

        inline bool prepare_jumps();

        inline bool next_instruction();

        inline void next_arg();

        void next_addr(bool calc_mem_addr = false);

        sljit_sw exec_offset = 0;

        instruction ip_temp;

        struct hanging_jump { 
            
            instruction ip_jump, ip_target;

            sljit_jump* jump = nullptr;
            sljit_label* target = nullptr;
            
        };

        struct rewritable_function_call {

            instruction ip_jaddr, ip_callee;

            sljit_jump* jump = nullptr;
            instruction ip_jaddr_next = -1;

        };

        sljit_compiler* compiler;

        inst_info cur_inst, last_inst;

        sljit_sw cur_part_index, cur_addr_index;

        int cur_scope;

        static vec<hanging_jump> hanging_jumps;

        static vec<rewritable_function_call> rewritable_function_calls;

    };

}
#include <iostream>
inline bool method_jit::jit::prepare_jumps() {

    if (!Eval::inst_initialized) Eval::inst_init();
    //std::cout << "Preparing jumps: ip=" << (ip-1) << "\n";
    auto cur_ip = ip - 1;
    auto inst_type = program[cur_ip];
    auto inst_info_idx = Eval::inst_info_offsets[inst_type];
    auto num_args = Eval::inst_info[inst_info_idx];
    auto inst_name = Eval::inst_names[inst_type];

    if (inst_type == Eval::inst::scpbeg) cur_scope++;
    else if (inst_type == Eval::inst::scpend) { cur_scope--; if (cur_scope == 0) { ip = ip_temp; return false; } }

    else if (inst_type == Eval::inst::j || inst_type == Eval::inst::jc) hanging_jumps.push_back({hanging_jump{ cur_ip, program[ip] }});

    inst_info_idx++;

    for (int arg = 0; arg < num_args; arg++) {
        //std::cout << "\targ" << arg << ": ip=" << (ip-1) << "\n";
        if (Eval::inst_info[inst_info_idx + arg]) ip += (program[ip] % 16 <= Eval::access::imm) ? 2 : 3;
        else ip++;

    }

    ip++;
    return true;

}

inline bool method_jit::jit::next_instruction() {
    
    auto cur_ip = ip - 1;
    auto I = program[cur_ip];

    hanging_jump* cur_hj = nullptr;

    for (auto& hj : hanging_jumps) {

        if (cur_ip == hj.ip_jump) cur_hj = &hj;

        if (cur_ip == hj.ip_target) hj.target = sljit_emit_label(compiler);

    }
    //std::cout << "\tip=" << (ip-1) << "Processing instruction: " << I << "\n";
    switch (I) {

#define begin_case(thecase) case Eval::inst::thecase: cur_part_index = 0; cur_addr_index = 0
#define begin_cases(thecase1, thecase2) case Eval::inst::thecase1: case Eval::thecase2: cur_part_index = 0; cur_addr_index = 0

#define _arg(idx) cur_inst.parts[idx].arg
#define _addr(idx) cur_inst.parts[idx].addr
#define _addr_(idx) _addr(idx).args[0], _addr(idx).args[1]


    begin_case(exit);

        sljit_emit_return_void(compiler);

    end_case();


    begin_case(begfnc);

        next_arg();

    end_case();

    begin_case(j);

    {

        next_arg();

        if (cur_hj) cur_hj->jump = sljit_emit_jump(compiler, SLJIT_JUMP);

    }

    end_case();


    begin_case(jc);

    {

        next_arg();

        next_addr();

        if (cur_hj) {

            sljit_emit_op2(compiler, SLJIT_XOR, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R0, 0);

            sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));
            
            cur_hj->jump = sljit_emit_cmp(compiler, SLJIT_EQUAL, SLJIT_R1, 0, SLJIT_R0, 0);

        }

    }
        
    end_case();


    begin_case(call);

    {

        next_arg();

        next_arg();

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SP_REGISTER, 0, SLJIT_IMM, 8 * _arg(0).iarg);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_R0), 8, SLJIT_IMM, ip);

        auto potential_func = program[_arg(1).iarg + 1];
        
        if (potential_func != 0) sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1V(W), SLJIT_IMM, potential_func);

        else {

            auto call = sljit_emit_call(compiler, SLJIT_CALL | SLJIT_REWRITABLE_JUMP, SLJIT_ARGS1V(W));

            sljit_set_target(call, (sljit_uw)-1);

            auto ip_callee = _arg(1).iarg;

            int first_occurrence = -1;
            int last_occurrence = -1;

            for (int i = rewritable_function_calls.size() - 1; i >= 0; i--) {

                auto& rwfc = rewritable_function_calls[i];

                if (rwfc.ip_callee == ip_callee) {

                    if (first_occurrence < 0) first_occurrence = i;
                    last_occurrence = i;

                }

            }

            instruction ip_jaddr_next = (last_occurrence >= 0) ? rewritable_function_calls[last_occurrence].ip_jaddr : -1;
            if (first_occurrence >= 0) rewritable_function_calls[first_occurrence].ip_jaddr_next = ip;

            rewritable_function_calls.push_back(rewritable_function_call{ ip, ip_callee, call, ip_jaddr_next });

        }

    }

    ip += 3;
    end_case();


    begin_case(ret);

        sljit_emit_return_void(compiler);
        if (cur_scope == 1) return false;

    end_case();

    begin_case(scpbeg);

    {

        next_arg();

        cur_scope++;

    }

    end_case();


    begin_case(scpend);

        cur_scope--; //if (cur_scope == 0) std::cout << "\t\tGoing to return false\n";
        if (cur_scope == 0) return false;

    end_case();


    begin_case(inp);

    {

        auto jump = sljit_emit_cmp(compiler, SLJIT_EQUAL, SLJIT_MEM, (sljit_sw)&(first_read), SLJIT_IMM, 1);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS0V(), SLJIT_IMM, (sljit_sw)_getchar);

        auto label = sljit_emit_label(compiler);

        sljit_set_label(jump, label);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM, (sljit_sw)&(first_read), SLJIT_IMM, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS0V(), SLJIT_IMM, (sljit_sw)_getline);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)strbuffer);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, P), SLJIT_IMM, (sljit_sw)alloc_string);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(0), SLJIT_R0, 0);

    }

    end_case();


    begin_case(out);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1V(W), SLJIT_IMM, (sljit_sw)_out);

    }
    
    end_case();


    begin_case(tmrs);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS0V(), SLJIT_IMM, (sljit_sw)_tmrs);

    end_case();


    begin_case(tmrr);

    {

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS0(F64), SLJIT_IMM, (sljit_sw)_tmrr);
        
        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(0), SLJIT_FR0, 0);

    }
    
    end_case();


    begin_case(rand);

    {

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS0(W), SLJIT_IMM, (sljit_sw)_rand);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(0), SLJIT_R0, 0);

    }

    end_case();

    //case gc:

        // Garbage collection here
        //end_case();

    begin_case(halloc);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)alloc);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(hinit);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op2(compiler, SLJIT_MUL, SLJIT_S1, 0, SLJIT_R0, 0, SLJIT_IMM, 8);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_S1, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)alloc);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S2, 0, SLJIT_R0, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_S1, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2V(W, W), SLJIT_IMM, (sljit_sw)_hzero);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_S2, 0);

    }

    end_case();


    begin_case(copy);

    {

        next_arg();

        next_addr(true);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(2));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R2, 0, SLJIT_IMM, _arg(0).iarg);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS3V(W, P, W), SLJIT_IMM, (sljit_sw)_arrcopy);

    }

    end_case();


    begin_case(mov);

    {

        next_addr();
        
        next_addr();
        
        auto format = _addr(0).getFormat();

        if (format == inst_addr_format_decimal)
            sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(1), _addr_(0));

        else
            sljit_emit_op1(compiler, (format == inst_addr_format_byte) ? SLJIT_MOV_U8 : SLJIT_MOV, _addr_(1), _addr_(0));

    }

    end_case();


    begin_case(strlen);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)heap);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(0));

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_MEM1(SLJIT_R0), -8);

        sljit_emit_op2(compiler, SLJIT_SUB, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 1);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(arrlen);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)heap);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(0));

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_MEM1(SLJIT_R0), -8);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, 8);
        
        sljit_emit_op0(compiler, SLJIT_DIV_SW);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(arrcat);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)heap);

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_S1, 0, _addr_(0), SLJIT_R0, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S2, 0, SLJIT_MEM1(SLJIT_S1), -8);

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_S3, 0, _addr_(1), SLJIT_R0, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S4, 0, SLJIT_MEM1(SLJIT_S3), -8);

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SLJIT_S2, 0, SLJIT_S4, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)alloc);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_S1, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R2, 0, SLJIT_S2, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S1, 0, SLJIT_R0, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS3V(W, P, W), SLJIT_IMM, (sljit_sw)_arrcopy);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_S2, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, 8);

        sljit_emit_op0(compiler, SLJIT_DIV_SW);

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SLJIT_S1, 0, SLJIT_S2, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_S3, 0);

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R2, 0, SLJIT_S4, 0);

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS3V(W, P, W), SLJIT_IMM, (sljit_sw)_arrcopy);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_S1, 0);

    }

    end_case();


    begin_case(castid);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CONV_F64_FROM_SW, _addr_(1), _addr_(0));

    }

    end_case();


    begin_case(castdi);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CONV_SW_FROM_F64, _addr_(1), _addr_(0));

    }

    end_case();
    

    begin_case(castsi);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)typecast_string_integer);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();

    
    begin_case(castis);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)typecast_integer_string);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(castsd);

    {

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(F64, W), SLJIT_IMM, (sljit_sw)typecast_string_decimal);

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(1), SLJIT_FR0, 0);

    }

    end_case();

    
    begin_case(castds);

    {

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, SLJIT_FR0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, F64), SLJIT_IMM, (sljit_sw)typecast_decimal_string);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(castas);

    {

        next_addr();
        
        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS1(W, W), SLJIT_IMM, (sljit_sw)typecast_ascii_string);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(castai);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(castia);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    begin_case(addi);

    {

        next_addr();

        next_addr();

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(addd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop2(compiler, SLJIT_ADD_F64, SLJIT_FR0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(2), SLJIT_FR0, 0);

    }

    end_case();


    begin_case(adda);
    
    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2(compiler, SLJIT_ADD, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(subi);

    {

        next_addr();

        next_addr();

        sljit_emit_op2(compiler, SLJIT_SUB, SLJIT_R0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(subd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop2(compiler, SLJIT_SUB_F64, SLJIT_FR0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(2), SLJIT_FR0, 0);

    }

    end_case();


    begin_case(suba);
    
    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2(compiler, SLJIT_SUB, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(muli);

    {

        next_addr();

        next_addr();

        sljit_emit_op2(compiler, SLJIT_MUL, SLJIT_R0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(muld);

    {

        next_addr();

        next_addr();

        sljit_emit_fop2(compiler, SLJIT_MUL_F64, SLJIT_FR0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(2), SLJIT_FR0, 0);

    }

    end_case();


    begin_case(divi);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op0(compiler, SLJIT_DIV_SW);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(divd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop2(compiler, SLJIT_DIV_F64, SLJIT_FR0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(2), SLJIT_FR0, 0);

    }

    end_case();


    begin_case(andi);

    {

        next_addr();

        next_addr();

        sljit_emit_op2(compiler, SLJIT_AND, SLJIT_R0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(andb);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2(compiler, SLJIT_AND, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(ori);

    {

        next_addr();

        next_addr();

        sljit_emit_op2(compiler, SLJIT_OR, SLJIT_R0, 0, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(orb);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2(compiler, SLJIT_OR, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(modi);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op0(compiler, SLJIT_DIVMOD_SW);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R1, 0);

    }

    end_case();


    begin_case(powi);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)intpow);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case(); 


    begin_case(powd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, SLJIT_FR0, 0, _addr_(0));

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, SLJIT_FR1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(F64, F64, F64), SLJIT_IMM, (sljit_sw)pow);

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_MOV_F64, _addr_(2), SLJIT_FR0, 0);

    }

    end_case();


    begin_case(adds);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)stradd);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(eqi);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_Z, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(eqd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_cases(eqb, eqa);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_Z, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(neqi);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_Z, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_NOT_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(neqd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_NOT_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_NOT_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_cases(neqb, neqa);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_Z, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_NOT_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gti);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_GREATER, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_SIG_GREATER);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gtd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_GREATER, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_GREATER);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gta);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_GREATER, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_GREATER);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gtei);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_GREATER_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_SIG_GREATER_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gted);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_GREATER_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_GREATER_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gtea);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_GREATER_EQUAL, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_GREATER_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(lti);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_LESS, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_SIG_LESS);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(ltd);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_LESS, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_LESS);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(lta);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_LESS, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_LESS);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(ltei);

    {

        next_addr();

        next_addr();

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_LESS_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_SIG_LESS_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(lted);

    {

        next_addr();

        next_addr();

        sljit_emit_fop1(compiler, SLJIT_CMP_F64 | SLJIT_SET_F_LESS_EQUAL, _addr_(0), _addr_(1));

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_F_LESS_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(ltea);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R1, 0, _addr_(1));

        sljit_emit_op2u(compiler, SLJIT_SUB | SLJIT_SET_SIG_LESS_EQUAL, SLJIT_R0, 0, SLJIT_R1, 0);

        next_addr();

        sljit_emit_op_flags(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_LESS_EQUAL);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(eqs);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)streq);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(neqs);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)strneq);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gts);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)strgt);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(gtes);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)strgte);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(lts);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)strlt);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(ltes);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, _addr_(1));

        sljit_emit_icall(compiler, SLJIT_CALL, SLJIT_ARGS2(W, W, W), SLJIT_IMM, (sljit_sw)strlte);

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, _addr_(2), SLJIT_R0, 0);

    }

    end_case();


    begin_case(noti);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, -1);

        sljit_emit_op2(compiler, SLJIT_SUB, _addr_(1), SLJIT_R0, 0, _addr_(0));

    }

    end_case();


    begin_case(notb);

    {

        next_addr();

        next_addr();

        sljit_emit_op1(compiler, SLJIT_MOV_U8, SLJIT_R0, 0, _addr_(0));

        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, 1);

        sljit_emit_op2(compiler, SLJIT_SUB, SLJIT_R0, 0, SLJIT_R1, 0, SLJIT_R0, 0);

        sljit_emit_op1(compiler, SLJIT_MOV_U8, _addr_(1), SLJIT_R0, 0);

    }

    end_case();


    //default:

        //std::cout << "\tInstruction unrecognized...\n"; while(true);

    }
    
    return true;

}

inline void method_jit::jit::next_arg() {

    cur_inst.parts[cur_part_index].arg.iarg = program[ip];

    cur_part_index++;
    ip++;

}

#endif