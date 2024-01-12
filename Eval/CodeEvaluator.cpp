#include "./CodeEvaluator.hpp"
#include "./NodeEvaluator.hpp"
#include "./../AST/Builder/implementations/GlobalScopeBuilder.hpp"
#include "./../AST/Pattern/implementations/Body.hpp"
#include "./../AST/Builder/implementations/Builder_Body_Code.hpp"
#include "./../AST/CodePiece/CodePiece_implementations.hpp"
#include <iostream>
Eval::CodeEvaluator::CodeEvaluator(EvaluatorProgress* progress) : progress(progress) {
    
    auto header_sym = (AST::HeaderSymbol*)progress->header_sym;
    auto code_body = (AST::Builder_Body_Code*)progress->code_body;

    auto header_table = header_sym ? ((AST::Builder_Header*)header_sym->header)->table : nullptr;
    auto code_table = code_body->table;

    if (progress->needs_load) {

        loadSymbolTypes(code_table(), progress->variable_types);
        if (header_sym) loadSymbolTypes(header_table(), progress->argument_types);

        auto cur = progress->parent;

        while (cur) {

            loadSymbolTypes(((AST::Builder_Body_Code*)cur->code_body)->table(), cur->variable_types);
            cur = cur->parent;

        }

    }
    else progress->needs_load = true;

    eval_type = progress->eval_type;
    if (header_sym) is_structure = header_sym->ID == AST::SymbolID::Structure;
    local_offset = 2 + (int)is_structure;

    bool init_success = progress->code_piece ? initFromCodePiece() : initFromCodeBody();
    if (!init_success) return;

    if (is_structure) {

        auto sinst = AST::StructureInstantiation{ progress->header_sym, progress->variable_types };
        eval_type = AST::Type::fromStructure(sinst).ID;
        
    }

    auto arg_typelist_ID = AST::Type::fromTypeList(progress->argument_types).ID;
    
    if (progress->parent) progress->parent->needs_load = false;
    
    else {

        if (eval_type == AST::Type::Anything) eval_type = AST::Type::Nothing; 

        if (header_sym) {
            
            if (!is_structure) progress->bytecode->addInstruction(inst::mov, Address::imm(0).setDataType(eval_type), Address::local(2).setDataType(eval_type));
            progress->bytecode->addInstruction(inst::ret);

            auto& inst_info = header_sym->instantiations[arg_typelist_ID];

            inst_info = AST::HeaderSymbol::InstantiationInfo{ eval_type, true };
            inst_info.bytecode = progress->bytecode();

            Evaluator::bytecode_blocks.push_back(progress->bytecode);

        }
        else progress->bytecode->addInstruction(inst::exit);

    }

    if (!progress->is_else_code) progress->bytecode->addInstruction(inst::scpend);

    progress->eval_type = eval_type;
    progress->owner->pop();

}

string Eval::CodeEvaluator::toString(int alignment) {

    return "";

}

void Eval::CodeEvaluator::loadSymbolTypes(void* symbol_table, AST::TypeList& types) {

    auto table = (AST::SymbolTable*)symbol_table;

    for (int i = 0; i < types.size(); i++) {

        auto& evaluator = table->variables[i]->evaluator;

        evaluator.eval_type = types[i];
        evaluator.is_constant = false;

    }

}

bool Eval::CodeEvaluator::initFromCodePiece() {

    auto code_piece = (AST::CodePiece*)progress->code_piece;

    evaluate(code_piece);

    if (error.error) return false;

    if (finished) {

        deferEvaluation(0);
        return false;

    }

    return true;

}

bool Eval::CodeEvaluator::initFromCodeBody() {

    auto header_sym = (AST::HeaderSymbol*)progress->header_sym;
    auto code_body = (AST::Builder_Body_Code*)progress->code_body;

    if (progress->progress < 0) {

        auto inst_info = 
            header_sym
                ? &header_sym->instantiations[AST::Type::fromTypeList(progress->argument_types).ID]
                : nullptr
            ;

        evaluateReturnType();

        if (error.error) return false;

        if (finished) {

            deferEvaluation(-1);
            return false;

        }

        progress->progress = 0;
        if (inst_info) inst_info->return_type = eval_type;

    }

    if (is_structure) progress->bytecode->addInstruction(
        inst::hinit,
        Address::imm(progress->num_vars[1]),
        Address::local(2)
    );

    auto& pieces = code_body->code_pieces.pieces;

    for (int i = progress->progress; i < pieces.size(); i++) {

        evaluate(pieces[i]());

        if (error.error) return false;

        if (finished) {

            deferEvaluation(i);
            return false;

        }
        else progress->sub_progress = 0;

    }

    return true;

}

void Eval::CodeEvaluator::evaluateReturnType() {

    if (!progress->header_sym) return;

    auto header_sym = (AST::HeaderSymbol*)progress->header_sym;
    auto return_exp = (AST::Expression*)header_sym->return_expression;
    
    if (!return_exp) return;

    NodeEvaluator evaluator(return_exp, false, false, true);

    if (evaluator.error.error) error = evaluator.error;

    else if (!AST::Type(evaluator.eval_type).isConcrete()) {

        error.error = true;
        error.info = "Return type must be concrete.";
        error.sources.push_back(new PrintableString("Source expression:\n" + return_exp->print(2, false)));

    }

    else if (evaluator.eval_type == AST::Type::Unknown) finished = true;

    else eval_type = evaluator.eval_type;

}

void Eval::CodeEvaluator::deferEvaluation(int i) {

    Evaluator::next_eval->owner = progress->owner;

    progress->progress = i;
    progress->owner->push(Evaluator::next_eval);

}

AST::TypeList _dummyList;

void Eval::CodeEvaluator::evaluate(AST::CodePiece* piece, bool inside_declaration) {

    auto pid = piece->ID;

    if (pid == AST::CodePieceID::PatternMatch) {

        auto cast = (AST::CodePiece_PatternMatch*)piece;
        auto pm = cast->pattern_match;

        Evaluator::cur_expression_info.lh_address = Address::local(progress->stack_offset + local_offset);
        Evaluator::cur_expression_info.cur_progress = progress;
        Evaluator::cur_expression_info.stack_offset = progress->stack_offset;

        NodeEvaluator evaluator(pm, true, false, true);
        
        if (evaluator.error.error) { error = evaluator.error; return; }

        if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

        if (!AST::Type(evaluator.eval_type).isConcrete()) {

            error.error = true;
            error.info = "Pattern match type must be concrete.";
            error.sources.push_back(new PrintableString("Source expression:\n" + pm->print(2, false)));

            return;

        }

        progress->bytecode->mergeWith(*evaluator.bytecode);

    }

    else if (pid == AST::CodePieceID::Declaration) {

        auto cast = (AST::CodePiece_Declaration*)piece;
        evaluate(cast->assignment(), true);

    }

    else if (pid == AST::CodePieceID::Assignment) {

        auto cast = (AST::CodePiece_Assignment*)piece;

        auto lh_sym = 
            (cast->LH_declare != nullptr) 
                ? cast->LH_declare
                : (
                    (cast->LH_assign->node->ID == AST::NodeID::Variable) 
                        ? ((AST::VariableNode*)cast->LH_assign->node())->sym() 
                        : nullptr
                )
            ;

        auto lh_exp = lh_sym ? nullptr : cast->LH_assign;

        Evaluator::cur_expression_info.cur_progress = progress;
        Evaluator::cur_expression_info.stack_offset = progress->stack_offset;

        if (lh_exp && !progress->lh_evaluated) {

            NodeEvaluator evaluator(lh_exp, true, true, true);

            if (evaluator.error.error) { error = evaluator.error; return; }

            if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

            progress->bytecode->mergeWith(*evaluator.bytecode);
            progress->lh_evaluated = true;
            progress->lh_type = evaluator.eval_type;
            progress->lh_address = Evaluator::cur_expression_info.lh_address;

        }

        if (progress->lh_evaluated) Evaluator::cur_expression_info.stack_offset += 2;

        if (is_structure) Evaluator::cur_expression_info.lh_address = Address::heap8(Address::local(2), Address::imm(lh_sym->index));
        
        else if (lh_sym) {

            auto lh_offset = _var_offset(lh_sym);
            Evaluator::cur_expression_info.lh_address = 
                lh_sym->is_global
                    ? Address::global(lh_offset)
                    : Address::local(lh_offset)
                ;

        }
        else Evaluator::cur_expression_info.lh_address = progress->lh_address;

        auto rh = cast->RH;

        NodeEvaluator evaluator(rh, true, false, true);

        if (evaluator.error.error) { error = evaluator.error; return; }

        if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

        if (evaluator.eval_type == AST::Type::Nothing) {
            
            error.error = true;
            error.info = 
                inside_declaration
                    ? "Variable cannot be initialized to an expression with no type."
                    : "Variable cannot be assigned an expression with no type."
                ;
            
            error.sources.push_back(new PrintableString("Problematic expression:\n" + rh->print(2, false)));

            return;

        }

        if (!AST::Type(evaluator.eval_type).isConcrete()) {

            error.error = true;
            error.info = "Variable assignment type must be concrete.";
            error.sources.push_back(new PrintableString("Source expression:\n" + rh->print(2, false)));

            return;

        }
        
        auto prev_type = lh_sym ? lh_sym->evaluator.eval_type : progress->lh_type;
        auto new_type = evaluator.eval_type;

        if (inside_declaration) {
            
            progress->variable_types.push_back(new_type);

            lh_sym->evaluator.eval_type = new_type;
            lh_sym->evaluator.is_constant = false;
        
        }

        else if (new_type != prev_type) {

            error.error = true;
            error.info = "Type mismatch in variable assignment.";

            ptr_Printable problem_exp = new PrintableString("Problematic expression:\n" + rh->print(2, false));

            auto problem_type_string = "Expression type:\n" + AST::Type(new_type).print(2, false);
            ptr_Printable problem_type = new PrintableString(problem_type_string);

            auto expected_type_string = "Expected type:\n" + AST::Type(prev_type).print(2, false);
            ptr_Printable expected_type = new PrintableString(expected_type_string);

            error.sources.push_back(problem_exp);
            error.sources.push_back(problem_type);
            error.sources.push_back(expected_type);

            return;

        }

        progress->bytecode->mergeWith(*evaluator.bytecode);
        progress->lh_evaluated = false;

    }

    else if (pid == AST::CodePieceID::Body) {

        if (consolidateWithChild()) return;

        auto cast = (AST::CodePiece_Body*)piece;
        auto body_type = cast->body->ID;

        auto code_body = 
            (body_type == AST::PatternID::Body_Function)
                ? ((AST::Body_Function*)cast->body)->builder
                : ((AST::Body_Structure*)cast->body)->builder
            ;

        newEvaluator(code_body(), nullptr, _dummyList, false);

        finished = true;

    }

    else if (pid == AST::CodePieceID::NestedCode) {
        
        auto cast = (AST::CodePiece_NestedCode*)piece;

        auto& sub_progress = progress->sub_progress;

        if (sub_progress == 0) {

            evaluate(cast->header());

            if (!error.error && !finished) sub_progress = 1;
            else return;

        }

        if (sub_progress == 1) {

            if (progress->const_condition_value == ConstantConditionValue::False) sub_progress = cast->body_pt2() ? 2 : 0;

            else if (cast->body_pt1->ID == AST::CodePieceID::Body) {

                evaluate(cast->body_pt1());

                if (!error.error && !finished) sub_progress = cast->body_pt2() ? 2 : 0;
                else return;

            }

            else if (consolidateWithChild()) sub_progress = cast->body_pt2() ? 2 : 0;

            else {

                newEvaluator(progress->code_body, cast->body_pt1(), progress->variable_types, false);

                finished = true;
                return;

            }

        }

        if (sub_progress == 2) {

            if (progress->const_condition_value == ConstantConditionValue::True) sub_progress = 0;

            else if (consolidateWithChild()) sub_progress = 0;

            else {

                newEvaluator(progress->code_body, cast->body_pt2(), progress->variable_types, true);

                finished = true;
                return;

            }

        }

        if (progress->const_condition_value == ConstantConditionValue::None) {

            if (cast->header()->ID == AST::CodePieceID::Else) {

                progress->bytecode->mergeWith(*progress->nest_body1);
                return;

            }

            int first_jump_offset = progress->nest_header_start + progress->nest_header->instructions.size();
            int first_jump_dest = first_jump_offset + 4 + progress->nest_body1->instructions.size();

            if (progress->nest_body2 != nullptr || progress->nest_header_is_while) first_jump_dest += 2;

            progress->bytecode->mergeWith(*progress->nest_header);
            progress->bytecode->addInstruction(inst::jc, 0, progress->nest_header_address);
            progress->bytecode->mergeWith(*progress->nest_body1);

            progress->bytecode->jumps.push_back(BytecodeBlock::HangingJump{ first_jump_offset, 1, nullptr, first_jump_dest });
            
            int second_jump_dest = 0;

            if (progress->nest_body2 != nullptr || progress->nest_header_is_while) {

                progress->bytecode->addInstruction(inst::j, 0);

                int second_jump_offset = first_jump_dest - 2;
                second_jump_dest = 
                    progress->nest_header_is_while
                        ? first_jump_offset - progress->nest_header->instructions.size()
                        : second_jump_offset + progress->nest_body2->instructions.size() + 2
                    ;

                progress->bytecode->jumps.push_back(BytecodeBlock::HangingJump{ second_jump_offset, 1, nullptr, second_jump_dest });

            }

            if (progress->nest_body2 != nullptr) progress->bytecode->mergeWith(*progress->nest_body2);

            if (progress->nest_header_is_while)
                for (auto& hj : progress->bytecode->jumps)
                    if (hj.jdest < 0) hj.jdest = hj.jdest == -1 ? second_jump_dest : first_jump_dest;

        }

        else if (progress->const_condition_value == ConstantConditionValue::True) {

            int jump_dest = progress->bytecode->instructions.size();

            progress->bytecode->mergeWith(*progress->nest_body1);

            if (progress->nest_header_is_while) {

                int jump_src = progress->bytecode->instructions.size();

                progress->bytecode->addInstruction(inst::j, 0);
                progress->bytecode->jumps.push_back(BytecodeBlock::HangingJump{ jump_src, 1, nullptr, jump_dest });

                for (auto& hj : progress->bytecode->jumps)
                    if (hj.jdest < 0) hj.jdest = hj.jdest == -1 ? jump_dest : progress->bytecode->instructions.size();

            }

        }

        else if (progress->nest_body2 != nullptr) progress->bytecode->mergeWith(*progress->nest_body2);

        progress->nest_body2 = nullptr;

    }

    //else if (pid == AST::CodePieceID::For)

    else if (pid == AST::CodePieceID::While) {

        auto cast = (AST::CodePiece_While*)piece;
        auto exp = cast->condition;

        Evaluator::cur_expression_info.lh_address = Address::local(progress->stack_offset + local_offset);
        Evaluator::cur_expression_info.cur_progress = progress;
        Evaluator::cur_expression_info.stack_offset = progress->stack_offset;

        NodeEvaluator evaluator(exp, true, false, false);

        if (evaluator.error.error) { error = evaluator.error; return; }

        if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

        if (evaluator.eval_type != AST::Type::Bool) {
            
            error.error = true;
            error.info = "'while' loop condition must be a boolean.";

            ptr_Printable problem_exp = new PrintableString("Problematic expression:\n" + exp->print(2, false));

            auto problem_type_string = "Expression type:\n" + AST::Type(evaluator.eval_type).print(2, false);
            ptr_Printable problem_type = new PrintableString(problem_type_string);

            error.sources.push_back(problem_exp);
            error.sources.push_back(problem_type);

            return;

        }

        if (evaluator.is_constant) progress->const_condition_value = evaluator.value<bool>() ? ConstantConditionValue::True : ConstantConditionValue::False;
        else progress->const_condition_value = ConstantConditionValue::None;
        
        progress->nest_header = evaluator.bytecode;
        progress->nest_header_is_while = true;
        progress->nest_header_start = progress->bytecode->instructions.size();
        progress->nest_header_address = evaluator.address;

    }

    else if (pid == AST::CodePieceID::If) {

        auto cast = (AST::CodePiece_If*)piece;
        auto exp = cast->condition;

        Evaluator::cur_expression_info.lh_address = Address::local(progress->stack_offset + local_offset);
        Evaluator::cur_expression_info.cur_progress = progress;
        Evaluator::cur_expression_info.stack_offset = progress->stack_offset;

        NodeEvaluator evaluator(exp, true, false, false);

        if (evaluator.error.error) { error = evaluator.error; return; }

        if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

        if (evaluator.eval_type != AST::Type::Bool) {
            
            error.error = true;
            error.info = "'if' statement condition must be a boolean.";

            ptr_Printable problem_exp = new PrintableString("Problematic expression:\n" + exp->print(2, false));

            auto problem_type_string = "Expression type:\n" + AST::Type(evaluator.eval_type).print(2, false);
            ptr_Printable problem_type = new PrintableString(problem_type_string);

            error.sources.push_back(problem_exp);
            error.sources.push_back(problem_type);

            return;

        }

        if (evaluator.is_constant) progress->const_condition_value = evaluator.value<bool>() ? ConstantConditionValue::True : ConstantConditionValue::False;
        else progress->const_condition_value = ConstantConditionValue::None;

        progress->nest_header = evaluator.bytecode;
        progress->nest_header_is_while = false;
        progress->nest_header_start = progress->bytecode->instructions.size();
        progress->nest_header_address = evaluator.address;

    }

    else if (pid == AST::CodePieceID::Return) {

        auto cast = (AST::CodePiece_Return*)piece;
        auto exp = cast->return_expression;

        Evaluator::cur_expression_info.lh_address = Address::local(2);
        Evaluator::cur_expression_info.cur_progress = progress;
        Evaluator::cur_expression_info.stack_offset = progress->stack_offset;

        NodeEvaluator evaluator(exp, true, false, true);

        if (evaluator.error.error) { error = evaluator.error; return; }

        if (evaluator.eval_type == AST::Type::Unknown) { finished = true; return; }

        if (!AST::Type(evaluator.eval_type).isConcrete()) {

            error.error = true;
            error.info = "Return expression type must be concrete.";
            error.sources.push_back(new PrintableString("Source expression:\n" + exp->print(2, false)));

            return;

        }

        if (eval_type == AST::Type::Anything || eval_type == evaluator.eval_type) { 
            
            eval_type = evaluator.eval_type;
            progress->eval_type = eval_type;

        }

        else {

            error.error = true;
            error.info = "Type mismatch in return expression.";

            ptr_Printable problem_exp = new PrintableString("Problematic expression:\n" + exp->print(2, false));

            auto problem_type_string = "Expression type:\n" + AST::Type(evaluator.eval_type).print(2, false);
            ptr_Printable problem_type = new PrintableString(problem_type_string);

            auto expected_type_string = "Expected type:\n" + AST::Type(eval_type).print(2, false);
            ptr_Printable expected_type = new PrintableString(expected_type_string);

            error.sources.push_back(problem_exp);
            error.sources.push_back(problem_type);
            error.sources.push_back(expected_type);

            return;

        }

        if (evaluator.eval_type != AST::Type::Nothing && !progress->header_sym) {

            error.error = true;
            error.info = "Main function cannot return a value.";

            ptr_Printable problem_exp = new PrintableString("Problematic expression:\n" + exp->print(2, false));
            error.sources.push_back(problem_exp);

        } 

        if (evaluator.eval_type != AST::Type::Nothing) progress->bytecode->mergeWith(*evaluator.bytecode);

        if (progress->header_sym) progress->bytecode->addInstruction(inst::ret);
        else progress->bytecode->addInstruction(inst::exit);

    }

    else if (pid == AST::CodePieceID::LoopLogic) {

        auto cast = (AST::CodePiece_LoopLogic*)piece;
        bool is_continue = cast->is_continue;

        progress->bytecode->jumps.push_back(BytecodeBlock::HangingJump{ (int)progress->bytecode->instructions.size(), 1, nullptr, is_continue ? -1 : -2 });
        progress->bytecode->addInstruction(inst::j, 0);

    }

}

void Eval::CodeEvaluator::newEvaluator(void* body, void* code_piece, AST::TypeList& var_types, bool is_else_code) {

    auto new_body = (AST::Builder_Body_Code*)body;
    auto new_table = new_body->table;

    ptr_EvaluatorProgress new_evaluator = new EvaluatorProgress{ };

    new_evaluator->argument_types = progress->argument_types;
    new_evaluator->variable_types = var_types;
    new_evaluator->code_body = body;
    new_evaluator->code_piece = code_piece;
    new_evaluator->header_sym = progress->header_sym;
    new_evaluator->eval_type = eval_type;
    new_evaluator->parent = progress;
    new_evaluator->needs_load = false;
    new_evaluator->stack_offset = progress->stack_offset + ((body == progress->code_body) ? 0 : new_table->variables.size());
    new_evaluator->num_vars = progress->num_vars;
    if (body != progress->code_body) new_evaluator->num_vars.push_back(new_table->variables.size() + progress->num_vars[progress->num_vars.size() - 1]);
    new_evaluator->bytecode = new BytecodeBlock();
    if (!is_else_code) new_evaluator->bytecode->addInstruction(inst::scpbeg, new_evaluator->stack_offset);
    new_evaluator->is_else_code = is_else_code;

    progress->cur_child = new_evaluator;

    Evaluator::next_eval = new_evaluator;

}

bool Eval::CodeEvaluator::consolidateWithChild() {

    if (progress->cur_child != nullptr) {
        
        if (eval_type == AST::Type::Anything) eval_type = progress->cur_child->eval_type;
        if (progress->cur_child->max_stack_growth > progress->max_stack_growth) progress->max_stack_growth = progress->cur_child->max_stack_growth;

        if (progress->sub_progress == 1) progress->nest_body1 = progress->cur_child->bytecode;
        else if (progress->sub_progress == 2) progress->nest_body2 = progress->cur_child->bytecode;
        else progress->bytecode->mergeWith(*progress->cur_child->bytecode);

        progress->cur_child = nullptr;

        return true;

    }

    return false;

}