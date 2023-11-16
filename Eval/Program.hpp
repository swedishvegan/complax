#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "./../util/CompileError.hpp"

/*

A Program starts with an input file and completes the entire
compilation process from start to finish, using all the
helper objects defined within this library. The steps are as
follows:

    (1) Open the file and do some preliminary parsing, i.e.
    detecting comments and computing scopes.

    (2) Generate the syntax tree for each file, using objects
    from the AST namespace. Files included or imported within
    the code are opened and processed as needed.

    (3) Perform type evaluation on the syntax tree, using
    objects from the Eval namespace.

    (4) Generate the resulting bytecode, also with the Eval
    namespace.

If at any point something goes wrong, a CompileError is
generated.

To learn more about how the compiler works, it is recommended
to walk through the codebase, starting on the includes of this
file and continuing down the include chain in a breadth-first
fashion. Most of the .hpp files in this library are fairly 
well-documented.

*/

namespace Eval {

    struct Program {

        CompileError error;

        double compilation_time = 0.0;

        Program(int, char**);

    protected:

        string input_filename;
        string output_filename;
        bool emit_syntax_tree = false;
        bool emit_bytecode = false;

        void parseArgs(int, char**);

    };

}

#endif