
#include "./Address.hpp"

Eval::Address Eval::Address::local(instruction arg1) {

    Address addr;

    addr.access_type = access::local;
    addr.args[0] = arg1;
    addr.num_args = 1;

    return addr;

}

Eval::Address Eval::Address::global(instruction arg1) {

    Address addr;

    addr.access_type = access::global;
    addr.args[0] = arg1;
    addr.num_args = 1;

    return addr;

}

Eval::Address Eval::Address::imm(instruction arg1) {

    Address addr;

    addr.access_type = access::imm;
    addr.args[0] = arg1;
    addr.num_args = 1;

    return addr;

}

Eval::Address Eval::Address::heap1(Address arg1, Address arg2) {

    Address addr;

    if (arg1.access_type == access::local) {

        if (arg2.access_type == access::local) addr.access_type = access::heap1ll;
        else if (arg2.access_type == access::global) addr.access_type = access::heap1lg;
        else addr.access_type = access::heap1li;

    }
    else {

        if (arg2.access_type == access::local) addr.access_type = access::heap1gl;
        else if (arg2.access_type == access::global) addr.access_type = access::heap1gg;
        else addr.access_type = access::heap1gi;

    }

    addr.args[0] = arg1.args[0];
    addr.args[1] = arg2.args[0];
    addr.num_args = 2;

    return addr;

}

Eval::Address Eval::Address::heap8(Address arg1, Address arg2) {

    Address addr;

    if (arg1.access_type == access::local) {

        if (arg2.access_type == access::local) addr.access_type = access::heap8ll;
        else if (arg2.access_type == access::global) addr.access_type = access::heap8lg;
        else addr.access_type = access::heap8li;

    }
    else {

        if (arg2.access_type == access::local) addr.access_type = access::heap8gl;
        else if (arg2.access_type == access::global) addr.access_type = access::heap8gg;
        else addr.access_type = access::heap8gi;

    }

    addr.args[0] = arg1.args[0];
    addr.args[1] = arg2.args[0];
    addr.num_args = 2;

    return addr;

}

Eval::Address& Eval::Address::setDataType(AST::TypeID eval_type) {

    instruction access_base = 0;
    if (eval_type == AST::Type::Decimal) access_base = 16;
    else if (eval_type == AST::Type::Bool || eval_type == AST::Type::Ascii) access_base = 32;
    
    access_type = (access_type % 16) + access_base; return *this;
    
}

Eval::Address::Address() { access_type = access::none; }