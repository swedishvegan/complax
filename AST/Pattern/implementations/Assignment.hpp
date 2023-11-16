#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#include "./Expression/Expression.hpp"

namespace AST {

    struct Assignment : public Pattern {

        ptr_Node node;

        Assignment();

        string getInfo(int);

    };

    using ptr_Assignment = ptr<Assignment>;

    struct Scanner_Assignment : public Scanner {

        Scanner_Assignment(Code::Loader& loader, int start, int end, int cur_best_start);

        static ptr_Expression expression;

    };

}

#endif