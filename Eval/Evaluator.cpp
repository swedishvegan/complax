#include "./Evaluator.hpp"
#include "./CodeEvaluator.hpp"
#include "./../AST/Symbol/Symbol.hpp"
#include "./../AST/Builder/implementations/GlobalScopeBuilder.hpp"
#include "./../AST/Pattern/implementations/Body.hpp"
#include "./../AST/CodePiece/CodePiece_implementations.hpp"

#define _finish() \
\
next_eval = nullptr; \
bytecode_blocks.clear(); \
\
return

Eval::Evaluator::Evaluator() {

    auto start_program = 
        AST::GlobalScopeBuilder::main_function 
            ? ((AST::Body_Function*)AST::GlobalScopeBuilder::main_function)->builder.cast<AST::Builder_Body_Code>()() 
            : nullptr;

    if (!start_program) {

        error.error = true;
        error.info = "Program has no starting point.";

        return;

    }

    auto table = start_program->table();
    auto num_vars = table->variables.size();

    is_evaluating = true;

    EvaluatorStack stack;

    ptr_EvaluatorProgress start_evaluator = new EvaluatorProgress{ };

    start_evaluator->code_body = start_program;
    start_evaluator->owner = &stack;
    start_evaluator->stack_offset = num_vars;
    start_evaluator->num_vars.push_back(0);
    start_evaluator->num_vars.push_back(num_vars);
    start_evaluator->bytecode = new BytecodeBlock();
    start_evaluator->bytecode->addInstruction(inst::scpbeg, start_evaluator->stack_offset);

    stack.push(start_evaluator);
    bytecode_blocks.push_back(start_evaluator->bytecode);

    while (stack.size() > 0) {

        CodeEvaluator evaluator(stack.top()());

        if (evaluator.error.error) { error = evaluator.error; _finish(); }

    }

    int offset = 0;

    for (auto bc : bytecode_blocks) {

        bc->global_index = offset;
        offset += bc->instructions.size();
        
    }

    for (auto bc : bytecode_blocks)

        for (auto& hj : bc->jumps)

            bc->instructions[hj.offset + hj.jump_value_offset] =
                hj.ref
                    ? ((BytecodeBlock*)((AST::HeaderSymbol::InstantiationInfo*)hj.ref)->bytecode)->global_index
                    : bc->global_index + hj.jdest;

    for (auto bc : bytecode_blocks) program_bytecode.mergeWith(*bc);

    _finish();

}

bool Eval::Evaluator::is_evaluating = false;

Eval::ptr_EvaluatorProgress Eval::Evaluator::next_eval;

Eval::ExpressionCompileInfo Eval::Evaluator::cur_expression_info;

Eval::ProgramDataTable Eval::Evaluator::program_data;

managed_vec<Eval::ptr_BytecodeBlock> Eval::Evaluator::bytecode_blocks;