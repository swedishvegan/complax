#include <cmath>
#include "./BytecodeBlock.hpp"
#include "./../AST/Symbol/Symbol.hpp"

Eval::BytecodeBlock::BytecodeBlock() { }

void Eval::BytecodeBlock::reset() { instructions.clear(); }

void Eval::BytecodeBlock::mergeWith(BytecodeBlock& block) { 

    for (auto hj : block.jumps) { 
        
        hj.offset += instructions.size(); 
        if (hj.jdest >= 0) hj.jdest += instructions.size();
        
        jumps.push_back(hj); 
        
    }
    for (auto in : block.instructions) instructions.push_back(in);

}

#define _max(a, b) ( (a > b) ? a : b )

void Eval::BytecodeBlock::call(void* ref, int num_args) {

    instructions.push_back(I(inst::call));
    instructions.push_back(I(num_args));
    instructions.push_back(I(0));
    instructions.push_back(I(0));
    instructions.push_back(I(0));
    instructions.push_back(I(0));

    auto hj = HangingJump { (int)instructions.size() - 6, 2, ref };
    jumps.push_back(hj);

}

void Eval::BytecodeBlock::addInstruction(instruction in) { 
    
    ibuf.push_back(I(in));

    for (auto in : ibuf) instructions.push_back(in);
    ibuf.clear();
    
}

void Eval::BytecodeBlock::addInstruction(Address addr) { 
    
    ibuf.push_back(I(addr.access_type));
    ibuf.push_back(addr.args[0]);
    if (addr.num_args == 2) ibuf.push_back(addr.args[1]);

    for (auto in : ibuf) instructions.push_back(in);
    ibuf.clear();
    
}

#define _line_number_str() ("\n" + indent(alignment + 1) + AST::pad("(" + std::to_string(idx) + ")", 11))

string Eval::BytecodeBlock::toString(int alignment) {

    if (instructions.size() == 0) return "Bytecode (empty)";

    if (!Eval::inst_initialized) Eval::inst_init();

    string s = "Bytecode";
    int idx = 0;

    while (idx < instructions.size()) {

        auto inst_type = instructions[idx];
        auto inst_info_idx = Eval::inst_info_offsets[inst_type];
        auto num_args = Eval::inst_info[inst_info_idx];
        auto inst_name = Eval::inst_names[inst_type];

        s += _line_number_str() + " " + string(inst_name) + "        ";

        inst_info_idx++;
        idx++;

        for (int arg = 0; arg < num_args; arg++) {

            if (Eval::inst_info[inst_info_idx + arg]) {

                auto access_dtype_num = instructions[idx] / 16;
                auto access_type = instructions[idx] % 16;
                auto access_name = Eval::access_names[access_type];
                auto access_dtype = (access_dtype_num == 2) ? "b." : ((access_dtype_num == 1) ? "d." : "i.");
                auto num_access_args = (access_type <= access::imm) ? 2 : 3;

                s += _line_number_str() + "               " + string(access_dtype) + string(access_name) + "      ";
                
                idx++;

                for (int sk = 1; sk < num_access_args; sk++) {

                    s += AST::pad(std::to_string(instructions[idx]), 7) + " ";
                    idx++;

                }

            }
            
            else {

                s += _line_number_str() + "               " + AST::pad(std::to_string(instructions[idx]), 7);
                idx++;

            }

        }

    }

    return s;

}