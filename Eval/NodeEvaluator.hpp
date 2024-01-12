#ifndef NODEEVALUATOR_HPP
#define NODEEVALUATOR_HPP

#include "./../AST/Type/Type.hpp"
#include "./../util/CompileError.hpp"
#include "./BytecodeBlock.hpp"
#include "./Address.hpp"

/*

NodeEvaluators analyze a Node and attempt to determine its data type. If it is unable to resolve a
data type, its eval_type attribute defaults to Type::Nothing. It also checks if the Node is a 
compile-time constant, and if it is, it evaluates the Node.

*/

#define _local_var_offset(thesym) (thesym->is_argument ? (thesym->index - Evaluator::cur_expression_info.cur_progress->argument_types.size()) : (Evaluator::cur_expression_info.cur_progress->num_vars[thesym->scope - 1] + thesym->index + 2))
#define _var_offset(thesym) (thesym->is_global ? Evaluator::program_data.getStackPosition(thesym) : _local_var_offset(thesym))

namespace Eval {

	struct ProgramDataTable;
	struct NodeEvaluator;

	using ptr_NodeEvaluator = ptr<NodeEvaluator>;

	struct NodeEvaluator : public Printable {

		CompileError error;

		AST::TypeID eval_type = AST::Type::Nothing;
		bool is_constant = false;

		ptr_BytecodeBlock bytecode; // Generated bytecode; will be empty if a NodeEvaluator is created during the AST generation stage of compilation
		Address address;            // Info on how to access the data referenced by this node

		template <typename T>
		T& value() { if (std::is_same<T, managed_string>().value) return *(T*)(&eval_str); return *((T*)eval); }

		double getNumericValue();

		NodeEvaluator();

		// Note: Many function arguments are void* in order to avoid mutual inclusions

		NodeEvaluator(void* expression, bool needs_val, bool is_lh, bool is_top_level); // If needs_val = false, the NodeEvaluator will only evaluate the type of the expression

		NodeEvaluator(void* node, void* parent_expression, bool needs_val, bool is_lh, bool is_top_level);

		string toString(int alignment);

	protected:

		union { char eval[8]; double _dummy; }; // If the Node is a numeric or boolean constant, its value is stored here (dummy is used to make sure struct has 8-byte alignment)
		managed_string eval_str;                // If the Node is a string constant, its value is stored here

		void* nd = nullptr;
		void* exp = nullptr;
		bool needs_val;
		bool is_lh;
		bool is_top_level;
		int stack_offset;

		NodeEvaluator(void*, void*, bool needs_val, bool is_lh, bool is_top_level, int stack_offset); // Last argument signifies whether the NodeEvaluator is working for a parent NodeEvaluator

		void construct(void*, void*);

		void evaluateArrayInitializer(void* ai); // ai is an AST::ArrayInitializerNode*

		void evaluateLiteral(void* lt);          // lt is an AST::LiteralNode*

		void evaluateVariable(void* vr);         // vr is an AST::VariableNode*

		void evaluatePatternMatch(void* pm);     // pm is an AST::PatternMatchNode*

		void evaluateStructureMember(void* sm); // sm is an AST::StructureMemberNode*

		void evaluateBuiltInPatternMatch(void* vsym, ptr_NodeEvaluator lh_eval, ptr_NodeEvaluator rh_eval); // vsym is an AST::HeaderSymbol*

		void evaluateGeneralPatternMatch(void* vsym, AST::TypeList&, managed_vec<ptr_NodeEvaluator>&);

		void* getRestrictions(void* header_sym); // Returns AST::Restrictions*

		void* findSuitableMatch(void* sym_bundle, AST::TypeList& restrictions); // Returns an AST::HeaderSymbol* of a suitable pattern match within a given bundle given the specified restrictions

		template <typename T>
		T binop(T lh, T rh, char op, AST::TypeID dtype);

		bool strComp(managed_string&, managed_string&, char);

		void zeroMemory();

		void setEvalType(AST::TypeID);

		void copy(NodeEvaluator&);

		friend struct ProgramDataTable;

	};

}

#endif