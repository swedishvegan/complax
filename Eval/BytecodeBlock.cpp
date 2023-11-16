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

    if (block.max_stack_growth > max_stack_growth) max_stack_growth = block.max_stack_growth;
    
}

void Eval::BytecodeBlock::call(void* ref, int num_args) {

    instructions.push_back(I(inst::call));
    instructions.push_back(I(num_args));
    instructions.push_back(I(0));

    auto hj = HangingJump { (int)instructions.size() - 3, 2, ref };
    jumps.push_back(hj);

}

int _inst_lengths[] = {

    0,
    1,
    1,
    2,
    2,
    0,
    1,
    1,
    0,
    1,
    1,
    -1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    2,
    2

};

const char* _inst_names[] = {

    "exit  ",
    "sass  ",
    "j     ",
    "jc    ",
    "call  ",
    "ret   ",
    "inp   ",
    "out   ",
    "tmrs  ",
    "tmrr  ",
    "rand  ",
    "gc    ",
    "movll ",
    "movpl ",
    "movxl ",
    "movlx ",
    "movpx ",
    "movxx ",
    "castid",
    "castdi",
    "castib",
    "castbi",
    "castdb",
    "castbd",
    "castsi",
    "castis",
    "castsd",
    "castds",
    "castsb",
    "castbs",
    "addi  ",
    "addd  ",
    "subi  ",
    "subd  ",
    "muli  ",
    "muld  ",
    "divi  ",
    "divd  ",
    "andi  ",
    "andb  ",
    "ori   ",
    "orb   ",
    "modi  ",
    "powi  ",
    "powd  ",
    "adds  ",
    "eqi   ",
    "eqd   ",
    "eqb   ",
    "neqi  ",
    "neqd  ",
    "neqb  ",
    "gti   ",
    "gtd   ",
    "gtei  ",
    "gted  ",
    "lti   ",
    "ltd   ",
    "ltei  ",
    "lted  ",
    "eqs   ",
    "neqs  ",
    "gts   ",
    "gtes  ",
    "lts   ",
    "ltes  ",
    "noti  ",
    "notb  "

};

#define _inst_length(in) (_inst_lengths[in])
#define _inst_name(in) (_inst_names[in])

string Eval::BytecodeBlock::toString(int alignment) {

    if (instructions.size() == 0) return "Bytecode (empty)";

    string s = "Bytecode";
    int idx = 0;

    while (idx < (int)instructions.size()) {

        auto in = instructions[idx];

        auto this_len = _inst_length(in);
        auto this_name = _inst_name(in);

        s += "\n" + indent(alignment + 1) + AST::pad("(" + std::to_string(idx) + ")", 11) + " " + this_name + "      ";

        for (int i = 0; i < this_len; i++) {

            idx++;
            s += AST::pad(std::to_string(instructions[idx]), 7) + " ";

        }

        idx++;

    }

    return s;

}

#define _is_between(i1, i2) in >= i1 && in <= i2
#define _check_arg(i) if (instructions[idx + i] > max_stack_growth) max_stack_growth = instructions[idx + i];

void Eval::BytecodeBlock::processInstruction() {

    int idx = instructions.size();

    for (auto in : ibuf) instructions.push_back(in);
    ibuf.clear();

    auto in = instructions[idx];
    auto this_len = _inst_length(in);

    if (in == inst::jc) { _check_arg(2); return; }
    if (in == inst::call) { _check_arg(1); return; }
    if (_is_between(inst::inp, inst::out)) { _check_arg(1); return; }
    if (in == inst::tmrr) { _check_arg(1); return; }
    if (in == inst::movll) { _check_arg(1); }
    if (_is_between(inst::movll, inst::movxl)) { _check_arg(2); return; }
    if (in == inst::movlx) { _check_arg(1); return; }
    if (_is_between(inst::castid, inst::castbs)) { _check_arg(1); _check_arg(2); return; }
    if (_is_between(inst::addi, inst::ltes)) { _check_arg(1); _check_arg(2); _check_arg(3); return; }
    if (_is_between(inst::noti, inst::notb)) { _check_arg(1); _check_arg(2); }

}

void Eval::BytecodeBlock::eliminateRedundantMov(int starting_idx) {
    /*
    if (starting_idx < 0) return;

    instruction i1 = instructions[starting_idx];
    if (i1 < I(inst::movll) || i1 > I(inst::movxx)) return;

    auto in = (inst)i1;

    if (in == inst::movll || in == inst::movxx)

    i1 -= I(inst::movll);
    i2 -= I(inst::movll);

    bool i1_dst_local = i1 < 3;
    bool i1_dst_direct = i1 >= 3;

    bool i2_src_local = i2 % 3 == 0;
    bool i2_src_direct = i2 % 3 == 2;
    
    movll,    // Arguments: 2,    Purpose: moves the 8-byte value at SP + arg0 to SP + arg1
    movpl,    // Arguments: 2,    Purpose: moves the 8-byte value arg0 to SP + arg1
    movxl,    // Arguments: 2,    Purpose: moves the 8-byte value at arg0 to SP + arg1
    movlx,    // Arguments: 2,    Purpose: moves the 8-byte value at SP + arg0 to arg1
    movpx,    // Arguments: 2,    Purpose: moves the 8-byte value arg0 to arg1
    movxx,
    
    if ((i1_dst_local && i2_src_local) || (i1_dst_direct && i2_src_direct)) {

        auto inew = 3 * (i2 / 3) + i1 % 3;
        inst in = (inst)(inew + I(inst::movll));

        instructions[starting_idx] = I(in);
        instructions[starting_idx + 2] = instructions[starting_idx + 5];

    }
*/
}