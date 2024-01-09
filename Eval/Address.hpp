#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include "./Instruction.hpp"
#include "./../AST/Type/Type.hpp"

// Helper object for generating bytecode instructions

namespace Eval {

    struct Address {

        static Address local(instruction);
        static Address global(instruction);
        static Address imm(instruction);
        static Address heap1(Address, Address);
        static Address heap8(Address, Address);

        Address& setDataType(AST::TypeID eval_type);

        instruction access_type;
        instruction args[2];
        int num_args;

        Address();

    };

}

#endif