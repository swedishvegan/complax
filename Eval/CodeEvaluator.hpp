#ifndef CODEEVALUATOR_HPP
#define CODEEVALUATOR_HPP

// Evaluates templated code bodies from Builder_Body_Code objects and outputs runnable bytecode

#include "./Evaluator.hpp"
#include "./../AST/CodePiece/CodePiece.hpp"

namespace Eval {

    struct CodeEvaluator : public Printable {

        CodeEvaluator(EvaluatorProgress*);

        CompileError error;

        string toString(int alignment);

        AST::TypeID eval_type = AST::Type::Anything; // Return type if source_body is a function, structure type if source_body is a structure

    protected:

        EvaluatorProgress* progress;

        bool finished = false;
        enum ConstantConditionValue { None, True, False }; // Used to skip over unreachable code blocks

        void loadSymbolTypes(void* symbol_table, AST::TypeList& types);

        bool initFromCodePiece(); // Returns false if evaluation was not able to complete

        bool initFromCodeBody(); // Same as above

        void evaluateReturnType();

        void deferEvaluation(int);

        void addInstantiation();

        void evaluate(AST::CodePiece*, bool inside_declaration = false);

        void newEvaluator(void* body, void* code_piece, AST::TypeList& var_types);

        bool consolidateWithChild();

    };

}

#endif