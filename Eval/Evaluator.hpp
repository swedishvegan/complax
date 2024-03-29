#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "./../util/vstack.hpp"
#include "./../util/CompileError.hpp"
#include "./../AST/Type/Type.hpp"
#include "./BytecodeBlock.hpp"
#include "./ProgramDataTable.hpp"
#include "./Address.hpp"

/*

Type evaluation is the next step in the compilation process once the syntax tree
has been generated. Since types are not declared explicitly in the language, the
compiler must deduct the types of all symbols. It does this by starting at the 
main function, and sequentially analyzing each CodePiece found in the main code 
body and performing type deduction on the RH expressions in assignments. Whenever 
a function is called, the evaluator recursively visits this function code body 
and walks through its contained CodePieces in order to determine its return type. 
It continues in this fashion until it has visited all reachable function and 
structure bodies within the program.

There is one major flaw to this approach -- if the codebase was sufficiently
large, it would be fairly easy to overflow the stack due to the recursive nature
of the algorithm. An Evaluator acts as a workaround to utilizing the stack for 
recursive code evaluation by keeping track of its own heap-based stack, which has
a much larger size limit than the default stack offered to C/C++ programs.

NOTE: Only one EvaluatorProgress is meant to be created during the compilation
process.

*/

namespace Eval {

    struct EvaluatorProgress;

    using ptr_EvaluatorProgress = ptr<EvaluatorProgress>;
    using EvaluatorStack = vstack<ptr_EvaluatorProgress>;

    struct EvaluatorProgress  {

        void* code_body = nullptr;
        void* header_sym = nullptr;

        AST::TypeList argument_types; // Types of arguments for this particular invocation
        AST::TypeList variable_types; // Types of the variables declared within the code body

        AST::TypeID eval_type = AST::Type::Anything; // Result of evaluation (Type::Anything signifies that no return type was found)

        int progress = -1;                 // The index of the last CodePiece which was successfully evaluated before the CodeEvaluator was interrupted (-1 signifies that the return type outside the function body has not yet been evaluated)
        int sub_progress = 0;              // Some CodePieces require multiple sub-steps to be processed
        int const_condition_value = 0;     // Stores CodeEvaluator::constant_condition_value
        ptr_BytecodeBlock nest_header;     // Condition of an "if" or "while" CodePiece
        bool nest_header_is_while = false; // Whether nest_header is a "while" condition
        int nest_header_start = 0;         // Beginning (local) instruction of the current nest header, if there is one
        Address nest_header_address;       // Address of the condition evaluated by the nest header
        ptr_BytecodeBlock nest_body1;      // "if" or "while" code
        ptr_BytecodeBlock nest_body2;      // "else" code

        void* code_piece = nullptr;        // If a CodePiece is specified, the CodeEvaluator defers to evaluating only the specified CodePiece, instead of the entire CodePiece list within the code body

        EvaluatorStack* owner = nullptr;    

        ptr_EvaluatorProgress cur_child;        // Temporarily filled in when child scopes are being evaluated so that parent scopes have access to their evaluations
        EvaluatorProgress* parent = nullptr;    // Child scopes need information on the current evaluation progress of their parent scopes
        bool needs_load = true;                 // Whether or not the child should reload the current state of relevant type evaluations

        managed_vec<int> num_vars; // Number of variables (cumulative) within each scope containing the current code body
        int stack_offset = 0;      // How many variables are on the stack within this scope
        int max_stack_growth = 0;  // Maximum relative size that the stack ever grows to within this code body

        bool lh_evaluated = false; // Whether or not the left-hand expression in the most recent assignment has already been evaluated
        AST::TypeID lh_type = 0;   // Type of the above mentioned expression
        Address lh_address;        // Address of left-hand expression

        bool is_else_code = false; // Whether or not this code belongs to the "else" of an "if" statement

        ptr_BytecodeBlock bytecode;

    };

    struct ExpressionCompileInfo  { // Information that the NodeEvaluator needs to generate proper bytecode

        Address lh_address;                                 // Info on how to access the LH of the expression in memory

        EvaluatorProgress* cur_progress = nullptr;          // Current code body being processed

        int stack_offset = 0;                               // Current stack offset when generating instructions

    };

    struct Evaluator  {

        CompileError error;

        BytecodeBlock program_bytecode;                        // Final bytecode of entire program

        Evaluator();

        static bool is_evaluating;                             // Signifies to CodeEvaluator and NodeEvaluator whether bytecode should be generated; defaults to false and set to true once AST generation finishes

        static ptr_EvaluatorProgress next_eval;                // Stores the next item that needs to be added to the evaluator stack

        static ExpressionCompileInfo cur_expression_info;      // Info on the current expression being compiled; used by CodeEvaluator and NodeEvaluator

        static ProgramDataTable program_data;

        static managed_vec<ptr_BytecodeBlock> bytecode_blocks; // Bytecode of individual code bodies
        
    };

    using ptr_Evaluator = ptr<Evaluator>;

}

#endif