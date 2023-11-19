#include <cstdint>
#include <type_traits>
#include <cmath>
#include "./Evaluator.hpp"
#include "./NodeEvaluator.hpp"
#include "./CodeEvaluator.hpp"
#include "./../AST/Pattern/implementations/Expression/Expression.hpp"
#include "./../AST/Pattern/implementations/Literal.hpp"
#include "./../AST/Builder/implementations/Builder_Header.hpp"
#include "./../AST/Builder/implementations/Builder_Body_Code.hpp"
#include "./../AST/Builder/implementations/Builder_Body_Restrictions.hpp"

using Integer = int64_t;
using Decimal = double;
using Bool = bool;
using String = managed_string;

template <typename T> T stoi(String& s) { try { return (T)std::stoll(s.c_str()); } catch(...) { return (T)0; } }
template <typename T> T stof(String& s) { try { return (T)std::stold(s.c_str()); } catch(...) { return (T)0; } }

Decimal Eval::NodeEvaluator::getNumericValue() {

	if (!is_constant) return 0.0;

	if (eval_type == AST::Type::Integer) return (Decimal)value<Integer>();
	if (eval_type == AST::Type::Decimal) return value<Decimal>();

	return 0.0;

}

Eval::NodeEvaluator::NodeEvaluator() { zeroMemory(); }

Eval::NodeEvaluator::NodeEvaluator(void* vexp, bool needs_val) 
	: exp(vexp), needs_val(needs_val), is_top_level(true), stack_offset(Evaluator::cur_expression_info.cur_progress ? Evaluator::cur_expression_info.cur_progress->stack_offset : 0) {

	zeroMemory();
	construct(((AST::Expression*)vexp)->node(), vexp);

}

Eval::NodeEvaluator::NodeEvaluator(void* vnode, void* vexp, bool needs_val) 
	: exp(vexp), needs_val(needs_val), is_top_level(true), stack_offset(Evaluator::cur_expression_info.cur_progress ? Evaluator::cur_expression_info.cur_progress->stack_offset : 0) {

	zeroMemory();
	construct(vnode, vexp);

}

string Eval::NodeEvaluator::toString(int alignment) {

	string s = "CompileTimeValue:\n";
	
	if (is_constant) {

		s += indent(alignment + 1);
		if (eval_type == AST::Type::Bool) s += value<Bool>() ? "true\n" : "false\n";
		else if (eval_type == AST::Type::String) s += value<String>() + "\n";
		else if (eval_type == AST::Type::Integer) s += std::to_string(value<Integer>()) + "\n";
		else if (eval_type == AST::Type::Decimal) s += std::to_string(value<Decimal>()) + "\n";
		else s += "(nothing)\n";

	}
	else s += indent(alignment + 1) + "(unknown)\n";

	s += indent(alignment) + "Type:\n";
	s += AST::PrintableTypeList::TypeID_toString(eval_type, alignment + 1);

	return s.substr(0, s.size() - 1);

}

Eval::NodeEvaluator::NodeEvaluator(void* vnode, void* vexp, bool needs_val, bool is_top_level, int stack_offset) 
	: exp(vexp), needs_val(needs_val), is_top_level(is_top_level), stack_offset(stack_offset) { 

	zeroMemory();
	construct(vnode, vexp);

}

#define _get_dst_info() \
\
int dst_index = is_top_level ? (Evaluator::cur_expression_info.lh_index) : (stack_offset + 2); \
bool dst_is_local = !is_top_level || Evaluator::cur_expression_info.lh_access_mode == ExpressionCompileInfo::LH_AccessMode::Relative

#define _get_literal_src_encoding(neval) \
\
instruction mov_src; \
bool src_is_string = false; \
\
if (neval->eval_type == AST::Type::String) { \
	\
	src_is_string = true; \
	mov_src = Evaluator::program_data.getStackPosition(nd, neval->value<String>()); \
	\
} \
\
else { \
	\
	if (neval->eval_type == AST::Type::Integer) mov_src = I_dirty_cast(neval->value<Integer>()); \
	else if (neval->eval_type == AST::Type::Decimal) mov_src = I_dirty_cast(neval->value<Decimal>()); \
	else mov_src = I_dirty_cast(neval->value<Bool>()); \
	\
}

#define _gen_literal_bytecode() \
\
_get_dst_info(); \
_get_literal_src_encoding(this); \
\
inst in; \
\
if (dst_is_local) in = src_is_string ? inst::movxl : inst::movpl; \
else in = src_is_string ? inst::movxx : inst::movpx; \
\
bytecode->addInstruction(in, mov_src, dst_index)

void Eval::NodeEvaluator::construct(void* vnode, void* vexp) {

	nd = vnode;
	auto node = (AST::Node*)vnode;

	if (node == nullptr) return;

	auto nid = node->ID;

	if (nid == AST::NodeID::Expression) {

		NodeEvaluator eval(((AST::ExpressionNode*)node)->expression()->node(), vexp, needs_val, is_top_level, stack_offset);
		copy(eval);

		return;

	}

	else if (nid == AST::NodeID::Literal) {

		auto l = ((AST::LiteralNode*)node)->literal;

		eval_type = l->type;

		if (!needs_val) return;

		is_constant = true;

		if (eval_type == AST::Type::Bool) value<Bool>() = l->info[0] == 't' ? true : false;

		else if (eval_type == AST::Type::String) value<String>() = l->info;

		else if (eval_type == AST::Type::Integer) value<Integer>() = stoi<Integer>(l->info);

		else if (eval_type == AST::Type::Decimal) value<Decimal>() = stof<Decimal>(l->info);

		if (Evaluator::is_evaluating && needs_val) {

			bytecode = new BytecodeBlock();
			_gen_literal_bytecode();

		}

		return;

	}

	else if (nid == AST::NodeID::Variable) {

		auto v = ((AST::VariableNode*)node);
		auto sym = v->sym;
		
		copy(sym->evaluator);

		if (!needs_val && eval_type == AST::Type::Nothing) {

			error.error = true;
			error.info = "Illegal reference to a variable.";

			error.sources.push_back(new PrintableString("Source expression:\n" + ((AST::Expression*)exp)->print(2, false)));

			error.sources.push_back(new PrintableString("Problematic variable:"));

			auto ppm = (new PrintableString(sym->oneLineDescription().c_str()))->print(1, false);
			error.sources.push_back(new PrintableString(ppm));
			
			return;

		}

		if (Evaluator::is_evaluating && needs_val) {

			is_constant = false;

			bytecode = new BytecodeBlock();

			int src_index;
			bool src_is_local = true;

			_get_dst_info();

			if (sym->is_global) {

				src_is_local = false;
				src_index = Evaluator::program_data.getStackPosition(sym());

			}
			else src_index = _local_var_offset(sym);

			inst in;
			
			if (dst_is_local) in = src_is_local ? inst::movll : inst::movxl;
			else in = src_is_local ? inst::movlx : inst::movxx;

			bytecode->addInstruction(in, src_index, dst_index);

		}

		return;

	}

	else if (nid == AST::NodeID::PatternMatch) evaluatePatternMatch(node);

}

void Eval::NodeEvaluator::evaluatePatternMatch(void* vpm) {

	auto pm = (AST::PatternMatchNode*)vpm;
	
	auto bundle = pm->bundle;
	auto sym = bundle->firstSym();

	managed_vec<ptr_NodeEvaluator> child_evaluators;
	
	AST::TypeList arg_types;

	for (int i = 0; i < sym->num_arguments; i++) {

		int arg_index = sym->argument_indices[i];
		
		auto comp = pm->components[arg_index];

		ptr_NodeEvaluator evaluator = new NodeEvaluator(comp(), exp, needs_val, false, stack_offset + i);

		if (evaluator->error.error) { error = evaluator->error; return; }

		if (evaluator->eval_type == AST::Type::Unknown) { eval_type = AST::Type::Unknown; return; }
		
		if (evaluator->eval_type == AST::Type::Nothing) {

			error.error = true;
			error.info = "Expression with no type passed as an argument to a pattern match.";

			error.sources.push_back(new PrintableString("Source expression:\n" + ((AST::Expression*)exp)->print(2, false)));

			error.sources.push_back(new PrintableString("Problematic argument found in:"));

			auto ppm = ptr_Printable(new PrintableString(sym->getSignature().c_str() + string(" (argument #") + std::to_string(i + 1) + ")"))->print(1, false);
			error.sources.push_back(new PrintableString(ppm));
			
			return;

		}

		arg_types.push_back(evaluator->eval_type);	
		child_evaluators.push_back(evaluator);

	}

	AST::HeaderSymbol* match = nullptr;

	for (auto header_sym : bundle->syms) {

		auto rest = (AST::Restrictions*)getRestrictions(header_sym());

		if (!rest) return;
		if (eval_type == AST::Type::Unknown) return;

		bool is_candidate = true;

		if (!rest->isEmpty()) for (int i = 0; i < sym->num_arguments; i++) {

			auto& arg_rest = rest->operator[](i);

			bool arg_matches = false;
			for (auto type : arg_rest) if (type == arg_types[i]) { arg_matches = true; break; }

			if (!arg_matches) { is_candidate = false; break; }

		}

		if (is_candidate) {
			
			if (match) {

				error.error = true;
				error.info = "Ambiguous pattern match has multiple valid matches given the supplied arguments.";

				error.sources.push_back(new PrintableString("Source expression:\n" + ((AST::Expression*)exp)->print(2, false)));

				error.sources.push_back(new PrintableString("Problematic pattern match:"));

				auto ppm = ptr_Printable(new PrintableString(sym->getSignature().c_str()))->print(1, false);
				error.sources.push_back(new PrintableString(ppm));

				ptr_Printable supl_types = new PrintableString("Argument types supplied:\n" + (new AST::PrintableTypeList(arg_types))->print(2, false));
				error.sources.push_back(supl_types);

				auto match1_header = (AST::Builder*)match->header;
				auto match2_header = (AST::Builder*)header_sym->header;

				match1_header->single_line = true;
				match2_header->single_line = true;

				error.sources.push_back(new PrintableString("First valid match:"));
				error.sources.push_back(new PrintableString(match1_header->print(1, false)));

				error.sources.push_back(new PrintableString("Second valid match:"));
				error.sources.push_back(new PrintableString(match2_header->print(1, false)));
			
				return;

			}

			match = (AST::HeaderSymbol*)header_sym();

		}

	}

	if (!match) {

		error.error = true;
		error.info = "Pattern match has no valid matches given the supplied arguments.";
		
		error.sources.push_back(new PrintableString("Source expression:\n" + ((AST::Expression*)exp)->print(2, false)));

		error.sources.push_back(new PrintableString("Problematic pattern match:"));

		auto ppm = ptr_Printable(new PrintableString(sym->getSignature().c_str()))->print(1, false);
		error.sources.push_back(new PrintableString(ppm));

		ptr_Printable supl_types = new PrintableString("Argument types supplied:\n" + (new AST::PrintableTypeList(arg_types))->print(2, false));
		error.sources.push_back(supl_types);

		return;

	}

	if (match->description.size() == 0) {

		auto header_syms = ((AST::Builder*)match->header)->table;

		for (int i = 0; i < sym->num_arguments; i++) {

			auto& evaluator = header_syms->variables[i]->evaluator;

			evaluator.is_constant = false;
			evaluator.eval_type = arg_types[i];

		}

		evaluateGeneralPatternMatch(match, arg_types, child_evaluators);

	}
	else {

		auto lh = child_evaluators.size() > 0 ? child_evaluators[0] : nullptr;
		auto rh = child_evaluators.size() > 1 ? child_evaluators[1] : nullptr;

		evaluateBuiltInPatternMatch(match, lh, rh);

	}

}

#define _get_var_pos(hand, offset) \
\
int var_pos_##hand = stack_offset + 2 + offset; \
bool needs_copy_##hand = true; \
\
if (hand##_node->ID == AST::NodeID::Variable) { \
	\
	auto varnode = (AST::VariableNode*)hand##_node; \
	auto varsym = varnode->sym; \
	\
	if (!varsym->is_global) { \
		\
		var_pos_##hand = _local_var_offset(varsym); \
		needs_copy_##hand = false; \
		\
	} \
	\
} \
if (needs_copy_##hand) bytecode->mergeWith(*hand##_eval->bytecode)

#define _get_built_in_function_value(Dtype, Insttype) \
\
eval_type = AST::Type::Dtype; \
\
if (bytecode != nullptr) { \
	\
	_get_dst_info(); \
	\
	if (dst_is_local) \
		bytecode->addInstruction(inst::Insttype, dst_index); \
	\
	else { \
		\
		bytecode->addInstruction(inst::Insttype, stack_offset + 2); \
		bytecode->addInstruction(inst::movlx, stack_offset + 2, dst_index); \
		\
	} \
	\
}

void Eval::NodeEvaluator::evaluateBuiltInPatternMatch(void* vsym, ptr_NodeEvaluator lh_eval, ptr_NodeEvaluator rh_eval) {

	auto sym = (AST::HeaderSymbol*)vsym;

	auto lh_node = lh_eval != nullptr ? (AST::Node*)lh_eval->nd : nullptr;
	auto lh_type = lh_eval != nullptr ? lh_eval->eval_type : AST::Type::Nothing;
	auto lh_constant = lh_eval != nullptr ? lh_eval->is_constant : true;

	auto rh_node = rh_eval != nullptr ? (AST::Node*)rh_eval->nd : nullptr;
	auto rh_type = rh_eval != nullptr ? rh_eval->eval_type : AST::Type::Nothing;
	auto rh_constant = rh_eval != nullptr ? rh_eval->is_constant : true;
	
	bool is_binop = sym->num_arguments == 2;

	auto& desc = sym->description;

	bool is_type_eval = desc[0] == 'T' && desc[1] == 'y';
	bool is_operator = desc[0] == 'O' && desc[1] == 'p';
	bool is_comparator = desc[0] == 'C' && desc[1] == 'o';
	bool is_sys = desc[0] == 'S' && desc[1] == 'y';

	if (Evaluator::is_evaluating && needs_val) bytecode = new BytecodeBlock();

	if (is_sys) {

		auto& name = sym->operator[](0).name;

		if (name[0] == 'o' && bytecode != nullptr) {

			_get_var_pos(lh, 0);

			if (lh_type != AST::Type::String) {

				inst in;

				if (lh_type == AST::Type::Bool) in = inst::castbs;
				else if (lh_type == AST::Type::Integer) in = inst::castis;
				else in = inst::castds;

				bytecode->addInstruction(in, var_pos_lh, stack_offset + 2);
				var_pos_lh = stack_offset + 2;

			}

			bytecode->addInstruction(inst::out, var_pos_lh);

		}

		else if (name[0] == 'i') { _get_built_in_function_value(String, inp); }

		else if (name[0] == 's' && bytecode != nullptr) bytecode->addInstruction(inst::tmrs);

		else if (name[0] == 't') { _get_built_in_function_value(Decimal, tmrr); }

		else if (name[0] == 'r') { _get_built_in_function_value(Integer, rand); }

		return;

	}

	if (!is_type_eval && !is_operator && !is_comparator) return;

	if (lh_type == AST::Type::Nothing && !is_type_eval) return;

	if (is_binop && rh_type == AST::Type::Nothing) return;

	if (is_type_eval) {

		auto& name = sym->operator[](is_binop ? 1 : 0).name;

		if (name[0] == 't') {

			eval_type = lh_type;
			is_constant = true;

			if (eval_type == AST::Type::Integer) value<Integer>() = 0;
			else if (eval_type == AST::Type::Decimal) value<Decimal>() = 0;
			else if (eval_type == AST::Type::Bool) value<Bool>() = 0;
			else value<String>() = "";

		}

		else if (name[0] == 'i') {

			eval_type = AST::Type::Bool;
			is_constant = true;

			bool negated = name.size() > 2 && name[2] == 'n';

			bool evaluation = lh_type == rh_type;
			if (negated) evaluation = !evaluation;
			
			if (needs_val) value<Bool>() = evaluation;

		}

		else {

			if (rh_eval == nullptr) rh_type = AST::Type::String;

			eval_type = rh_type;

			if (lh_constant && rh_constant && needs_val) {
				
				is_constant = true;
				
#define _typecast_case_(ltype, rtype) else if (lh_type == AST::Type::ltype && rh_type == AST::Type::rtype) value<rtype>() = (rtype)lh_eval->value<ltype>()

#define _typecast_case(ltype) \
\
_typecast_case_(ltype, Bool); \
_typecast_case_(ltype, Integer); \
_typecast_case_(ltype, Decimal)

#define _typecast_to_string(ltype) else if (lh_type == AST::Type::ltype && rh_type == AST::Type::String) value<String>() = std::to_string(lh_eval->value<ltype>())

				if (false);
				_typecast_case(Bool);
				_typecast_case(Integer);
				_typecast_case(Decimal);
				_typecast_to_string(Integer);
				_typecast_to_string(Decimal);
				else if (lh_type == AST::Type::Bool && rh_type == AST::Type::String) value<String>() = lh_eval->value<Bool>() ? "true" : "false";
				else if (lh_type == AST::Type::String) {
					
					String& s = lh_eval->value<String>();

					if (rh_type == AST::Type::Bool) value<Bool>() = s == "true";
					else if (rh_type == AST::Type::Integer) value<Decimal>() = stoi<Decimal>(s);
					else if (rh_type == AST::Type::Decimal) value<Decimal>() = stof<Decimal>(s);
					else value<String>() = s;

				}

			}

			if (!is_constant && bytecode != nullptr) {
				
				_get_var_pos(lh, 0);
				_get_dst_info();

				if (lh_type == rh_type) {

					inst in = dst_is_local ? inst::movll : inst::movlx;
					bytecode->addInstruction(in, var_pos_lh, dst_index);

				}

				else {

					inst castin;

#define _typecast_inst_case(Ltype, Rtype, lt, rt) else if (lh_type == AST::Type::Ltype && rh_type == AST::Type::Rtype) castin = inst::cast##lt##rt

					if (false);
					_typecast_inst_case(Integer, Decimal, i, d);
					_typecast_inst_case(Integer, Bool, i, b);
					_typecast_inst_case(Integer, String, i, s);
					_typecast_inst_case(Decimal, Integer, d, i);
					_typecast_inst_case(Decimal, Bool, d, b);
					_typecast_inst_case(Decimal, String, d, s);
					_typecast_inst_case(Bool, Integer, b, i);
					_typecast_inst_case(Bool, Decimal, b, d);
					_typecast_inst_case(Bool, String, b, s);
					_typecast_inst_case(String, Integer, s, i);
					_typecast_inst_case(String, Decimal, s, d);
					_typecast_inst_case(String, Bool, s, b);

					if (dst_is_local)
						bytecode->addInstruction(castin, var_pos_lh, dst_index);

					else {

						bytecode->addInstruction(castin, var_pos_lh, stack_offset + 2);
						bytecode->addInstruction(inst::movlx, stack_offset + 2, dst_index);

					}

				}

			}

		}

		if (is_constant && bytecode != nullptr) { _gen_literal_bytecode(); }

		return;

	}

	char op;

	if (is_operator) op = !is_binop ? sym->operator[](0).name[0] : sym->operator[](1).name[0];
	else {

		auto& name = sym->operator[](1).name;

		if (name[0] == '=') op = name[0];
		else if (name[0] == '!') op = is_binop ? '!' : 'n';
		else if (name.size() == 1) op = (name[0] == '>') ? 'G' : 'L';
		else op = (name[0] == '>') ? 'g' : 'l';

	}

	if (lh_type == AST::Type::String) {

		if (rh_type == AST::Type::String) {

			if (op == '+') {

				eval_type = AST::Type::String;

				if (lh_constant && rh_constant && needs_val) {

					value<String>() = lh_eval->value<String>() + rh_eval->value<String>();
					is_constant = true;

				}

			}

			else if (op == 'G' || op == 'g' || op == 'L' || op == 'l' || op == '=' || op == '!') {

				eval_type = AST::Type::Bool;

				if (lh_constant && rh_constant && needs_val) {

					value<Bool>() = strComp(lh_eval->value<String>(), rh_eval->value<String>(), op);
					is_constant = true;

				}

			}

		}

	}
		
	else if (!is_binop) {

#define _unop_case(type) \
\
else if (lh_type == AST::Type::type) { \
	\
	eval_type = AST::Type::type; \
	\
	if (lh_constant) { \
		\
		value<type>() = binop(lh_eval->value<type>(), (type)0, op, eval_type); \
		is_constant = true; \
		\
	} \
	\
}

		if (false);
		_unop_case(Integer)
		_unop_case(Bool)

	}
	else {

#define _binop_case(ltype, rtype, etype) \
\
else if (lh_type == AST::Type::ltype && rh_type == AST::Type::rtype) { \
	\
	eval_type = is_operator ? AST::Type::etype : AST::Type::Bool; \
	\
	if (lh_constant && rh_constant && needs_val) { \
		\
		auto v = binop((etype)lh_eval->value<ltype>(), (etype)rh_eval->value<rtype>(), op, AST::Type::etype); \
		\
		if (eval_type != AST::Type::Nothing) { \
			\
			if (is_operator) value<etype>() = v; \
			else value<Bool>() = (Bool)v; \
			\
			is_constant = true; \
			\
		} \
		\
	} \
	\
}

		if (false);
		_binop_case(Integer, Integer, Integer)
		_binop_case(Integer, Decimal, Decimal)
		_binop_case(Decimal, Integer, Decimal)
		_binop_case(Decimal, Decimal, Decimal)
		_binop_case(Bool, Bool, Bool);

	}

	if (bytecode != nullptr) {

		if (is_constant) { _gen_literal_bytecode(); }

		else if (is_binop) {

			_get_var_pos(lh, 0);
			_get_var_pos(rh, 1);

			auto ret_type = lh_type;

			if (lh_type != rh_type) {

				bool lh_needs_cast = lh_type == AST::Type::Integer && rh_type == AST::Type::Decimal;

				if (lh_needs_cast) {

					bytecode->addInstruction(inst::castid, var_pos_lh, stack_offset + 2);
					var_pos_lh = stack_offset + 2;

					ret_type = AST::Type::Decimal;

				}
				else {

					bytecode->addInstruction(inst::castid, var_pos_rh, stack_offset + 3);
					var_pos_rh = stack_offset + 3;

				}

			}

			_get_dst_info();

			inst binopin = inst::exit;

#define _binop_inst_case(thetype, tt, opc, opname) else if (ret_type == AST::Type::thetype && op == opc) binopin = inst::opname##tt

			if (false);
			_binop_inst_case(Integer, i, '+', add);
			_binop_inst_case(Decimal, d, '+', add);
			_binop_inst_case(String, s, '+', add);
			_binop_inst_case(Integer, i, '-', sub);
			_binop_inst_case(Decimal, d, '-', sub);
			_binop_inst_case(Integer, i, '*', mul);
			_binop_inst_case(Decimal, d, '*', mul);
			_binop_inst_case(Integer, i, '/', div);
			_binop_inst_case(Decimal, d, '/', div);
			_binop_inst_case(Integer, i, '^', pow);
			_binop_inst_case(Decimal, d, '^', pow);
			_binop_inst_case(Integer, i, '&', and);
			_binop_inst_case(Bool, b, '&', and);
			_binop_inst_case(Integer, i, '|', or);
			_binop_inst_case(Bool, b, '|', or);
			_binop_inst_case(Integer, i, '%', mod);
			_binop_inst_case(Integer, i, '=', eq);
			_binop_inst_case(Decimal, d, '=', eq);
			_binop_inst_case(Bool, b, '=', eq);
			_binop_inst_case(String, s, '=', eq);
			_binop_inst_case(Integer, i, '!', neq);
			_binop_inst_case(Decimal, d, '!', neq);
			_binop_inst_case(Bool, b, '!', neq);
			_binop_inst_case(String, s, '!', neq);
			_binop_inst_case(Integer, i, 'G', gt);
			_binop_inst_case(Decimal, d, 'G', gt);
			_binop_inst_case(String, s, 'G', gt);
			_binop_inst_case(Integer, i, 'g', gte);
			_binop_inst_case(Decimal, d, 'g', gte);
			_binop_inst_case(String, s, 'g', gte);
			_binop_inst_case(Integer, i, 'L', lt);
			_binop_inst_case(Decimal, d, 'L', lt);
			_binop_inst_case(String, s, 'L', lt);
			_binop_inst_case(Integer, i, 'l', lte);
			_binop_inst_case(Decimal, d, 'l', lte);
			_binop_inst_case(String, s, 'l', lte);
			_binop_inst_case(Bool, b, 'a', and);
			_binop_inst_case(Bool, b, 'o', or);

			if (dst_is_local)
				bytecode->addInstruction(binopin, var_pos_lh, var_pos_rh, dst_index);

			else {

				bytecode->addInstruction(binopin, var_pos_lh, var_pos_rh, stack_offset + 2);
				bytecode->addInstruction(inst::movlx, stack_offset + 2, dst_index);

			}

		}
		
		else {

			_get_var_pos(lh, 0);
			_get_dst_info();

			inst in = lh_type == AST::Type::Bool ? inst::notb : inst::noti;

			if (dst_is_local)
				bytecode->addInstruction(in, var_pos_lh, dst_index);

			else {

				bytecode->addInstruction(in, var_pos_lh, stack_offset + 2);
				bytecode->addInstruction(inst::movlx, stack_offset + 2, dst_index);

			}

		}

	}

}

void Eval::NodeEvaluator::evaluateGeneralPatternMatch(void* vsym, AST::TypeList& arg_types, managed_vec<ptr_NodeEvaluator>& child_evaluators) {

	auto sym = (AST::HeaderSymbol*)vsym;
	auto& instantiations = sym->instantiations;

	auto arg_list_ID = AST::Type(arg_types).ID;
	auto find_ID = instantiations.find(arg_list_ID);

	if (find_ID == instantiations.end()) {
		
		auto new_body = (AST::Builder_Body_Code*)sym->body;

		if (!new_body) {

			error.error = true;
			error.info = "Definition not found for pattern match:";

			auto sym_header = (Printable*)sym->header;
			sym_header->single_line = true;

			error.sources.push_back(new PrintableString(sym_header->print(0, false)));

			return;

		}

		auto new_evaluator = new EvaluatorProgress{ };

		new_evaluator->argument_types = arg_types;
		new_evaluator->code_body = sym->body;
		new_evaluator->header_sym = sym;
		new_evaluator->stack_offset = new_body->table->variables.size();
		new_evaluator->num_vars.push_back(0);
		new_evaluator->num_vars.push_back(new_evaluator->stack_offset);
		new_evaluator->bytecode = new BytecodeBlock();
		new_evaluator->bytecode->addInstruction(inst::sass, 0);

		Evaluator::next_eval = new_evaluator;
		eval_type = AST::Type::Unknown;

		return;

	}

	auto& instantiation_info = find_ID->second;

	if (instantiation_info.return_type == AST::Type::Anything && !instantiation_info.evaluation_complete) {

		error.error = true;
		error.info = "Recursive functions with the same type instantiation must have their return types explicitly declared, and the return type must not reference itself.";

		error.sources.push_back(new PrintableString("Source expression:\n" + ((AST::Expression*)exp)->print(2, false)));

		error.sources.push_back(new PrintableString("Problematic pattern match:"));

		auto ppm = ptr_Printable(new PrintableString(sym->getSignature().c_str()))->print(1, false);
		error.sources.push_back(new PrintableString(ppm));

		ptr_Printable supl_types = new PrintableString("Argument types supplied:\n" + (new AST::PrintableTypeList(arg_types))->print(2, false));
		error.sources.push_back(supl_types);

		return;

	}

	eval_type = instantiation_info.return_type;

	if (Evaluator::is_evaluating && needs_val) {

		bytecode = new BytecodeBlock();
		int stack_growth = 2 + stack_offset + child_evaluators.size();

		for (auto e : child_evaluators) bytecode->mergeWith(*e->bytecode);
		bytecode->call(&instantiation_info, stack_growth);

		if (is_top_level && Evaluator::cur_expression_info.lh_access_mode == ExpressionCompileInfo::LH_AccessMode::None) return;

		_get_dst_info();

		inst in = dst_is_local ? inst::movll : inst::movlx;

		bytecode->addInstruction(in, stack_growth + 2, dst_index);

	}

}

void* Eval::NodeEvaluator::getRestrictions(void* vsym) {

	auto sym = (AST::HeaderSymbol*)vsym;

	if (sym->restrictions) {

		auto eval_progress = sym->restrictions_eval_progress;

		if (eval_progress == AST::HeaderSymbol::RestrictionsEvaluationProgress::Complete) return sym->restrictions;

		if (eval_progress == AST::HeaderSymbol::RestrictionsEvaluationProgress::InProgress) { 

			error.error = true;
			error.info = "Pattern match cannot refer to itself within its restrictions, either directly or indirectly.";

			auto sym_header = (AST::Builder_Header*)sym->header;
			sym_header->single_line = true;

			error.sources.push_back(new PrintableString("Problematic pattern match:\n" + sym_header->print(2, false)));

			return nullptr;

		}

	}
	
	auto rest = sym->restrictions ? sym->restrictions : new AST::Restrictions();
	sym->restrictions = rest;

	auto header_syms = ((AST::Builder_Header*)sym->header)->table;
	auto rest_source = (AST::Builder_Body_Restrictions*)sym->restrictions_source;

	if (!rest_source) return rest;

	sym->restrictions_eval_progress = AST::HeaderSymbol::RestrictionsEvaluationProgress::InProgress;

	for (int i = 0; i < sym->num_arguments; i++) {
		
		auto arg_sym = header_syms->variables[i]; 
		auto& rest_expressions = rest_source->restrictions[arg_sym];

		for (auto exp : rest_expressions) {

			Eval::NodeEvaluator evaluator(exp(), false);

			if (evaluator.error.error) { error = evaluator.error; return nullptr; }

			if (evaluator.eval_type == AST::Type::Unknown) { 
				
				eval_type = AST::Type::Unknown; 
				sym->restrictions_eval_progress = AST::HeaderSymbol::RestrictionsEvaluationProgress::None;
				
				return rest;
				
			}

			if (evaluator.eval_type == AST::Type::Nothing) {

				error.error = true;
            	error.info = "Type restriction cannot be an expression with no type.";
            	error.sources.push_back(new PrintableString("Problematic expression:\n" + exp->print(2, false)));

				return nullptr;

			}

			auto& types = rest->operator[](i);

			bool is_repeat = false;
			for (auto type : types) if (evaluator.eval_type == type) { is_repeat = true; break; }

			if (!is_repeat) types.push_back(evaluator.eval_type);

		}

	}

	sym->restrictions_eval_progress = AST::HeaderSymbol::RestrictionsEvaluationProgress::Complete;

	return rest;

}

Integer ipow(Integer base, Integer exp) {

	Integer result = 1;

	while (exp > 0) {

		if (exp % 2) result *= base;

		exp /= 2;
		base *= base;

	}

	return result;

}

template <typename T>
T Eval::NodeEvaluator::binop(T lh, T rh, char op, AST::TypeID dtype) {

#define _binop_eval__(Opt, opt, condition, cast) \
\
else if (op == Opt) { \
	\
	if (condition) return (T)((cast)lh opt (cast)rh); \
	\
	eval_type = AST::Type::Nothing; \
	return (T)0; \
	\
}

#define _binop_eval_(opt, condition, cast) _binop_eval__((#opt)[0], opt, condition, cast)

#define _binop_eval(opt, condition) _binop_eval_(opt, condition, T)

	if (false);
	_binop_eval(+, dtype != AST::Type::Bool)
	_binop_eval(-, dtype != AST::Type::Bool)
	_binop_eval(*, dtype != AST::Type::Bool)
	_binop_eval_(&, dtype == AST::Type::Integer, Integer)
	_binop_eval_(|, dtype == AST::Type::Integer, Integer)
	_binop_eval__('G', >, dtype != AST::Type::Bool, T)
	_binop_eval__('L', <, dtype != AST::Type::Bool, T)
	_binop_eval__('g', >=, dtype != AST::Type::Bool, T)
	_binop_eval__('l', <=, dtype != AST::Type::Bool, T)
	_binop_eval__('=', ==, true, T)
	_binop_eval__('!', !=, true, T)

	if (op == '/') {

		if (dtype == AST::Type::Integer) return (T)((Integer)lh / (Integer)rh);
		if (dtype != AST::Type::Bool) return (T)((Decimal)lh / (Decimal)rh);

		return (T)0;
		eval_type = AST::Type::Nothing;

	}

	if (op == '^') {

		if (dtype == AST::Type::Integer) return rh < (T)0 ? (T)0 : (T)ipow((Integer)lh, (Integer)rh);
		if (dtype != AST::Type::Bool) return (T)pow((Decimal)lh, (Decimal)rh);

		return (T)0;
		eval_type = AST::Type::Nothing;
		
	}
	if (op == '%') {

		if (dtype == AST::Type::Integer) return (T)((Integer)lh % (Integer)rh);

		return (T)0;
		eval_type = AST::Type::Nothing;
		
	}
	if (op == 'n') {

		if (dtype == AST::Type::Integer || dtype == AST::Type::Bool) return (T)(!(Integer)lh);

		eval_type = AST::Type::Nothing;
		return (T)0;

	}
	if (dtype != AST::Type::Bool) { eval_type = AST::Type::Nothing; return (T)0; }
	if (op == 'a') return (T)((Bool)lh && (Bool)rh);
	if (op == 'o') return (T)((Bool)lh || (Bool)rh);
	return (T)(!(Bool)lh);

}

bool Eval::NodeEvaluator::strComp(String& lh, String& rh, char op) {

	if (op == 'G') return lh > rh;
	if (op == 'L') return lh < rh;
	if (op == 'g') return lh >= rh;
	if (op == 'l') return lh <= rh;
	if (op == '=') return lh == rh;
	return lh != rh;

}

void Eval::NodeEvaluator::zeroMemory() { for (int i = 0; i < 8; i++) eval[i] = (char)0; }

void Eval::NodeEvaluator::copy(NodeEvaluator& cev) { 

	error = cev.error;
	
	if (cev.eval_type > 0) { 
		
		if (cev.eval_type == AST::Type::String) eval_str = cev.eval_str;
		else for (int i = 0; i < 8; i++) eval[i] = cev.eval[i];
	
	} 
	
	eval_type = cev.eval_type; 
	is_constant = cev.is_constant;
	bytecode = cev.bytecode;

}