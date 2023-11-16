#include "./Builder_Body_Precedence.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/FreePattern.hpp"
#include "./../../Pattern/implementations/Assignment.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"
#include "./../../../Eval/NodeEvaluator.hpp"

AST::Builder_Body_Precedence::Builder_Body_Precedence(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols)
	: Builder(BuilderID::Builder_Body_Precedence, loader, start, end) {

	this->main_scope = scope;
	this->symbols = symbols.linkWith(table);
	this->variable_search_tree = symbols.variable_search_tree.cast<VariableSearchTree>();

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::None);

	build();
	Scanner_Expression::cleanup();

}

string AST::Builder_Body_Precedence::printChildren(int alignment) {
	
	string s;

	s += indent(alignment) + "p0: " + std::to_string(sym_precedence.p0) + "\n";
	s += indent(alignment) + "p1: " + std::to_string(sym_precedence.p1) + "\n";
	s += indent(alignment) + "p2: " + std::to_string(sym_precedence.p2);

	return s;

}

AST_DECL_SUCCESSORS(Builder_Body_Precedence) {

	if (exp_count == 0) AST_SUCC_ALLOW(Expression);
	else {

		if (exp_count < 3) AST_SUCC_ALLOW(Expression);
		AST_SUCC_ALLOW(Done);

	}

}

bool AST::Builder_Body_Precedence::processPattern(ptr_Pattern p) {

	auto exp = p.cast<Expression>();
	Eval::NodeEvaluator evaluator(exp(), true);

	if (evaluator.error.error) {

		error = evaluator.error;

		finished = true;
		return false;

	}

	if (!evaluator.is_constant) {

		error.error = true;
		error.info = "Precedence expression must be compile-time constant:";
		error.sources.push_back(exp.cast<Printable>());

		finished = true;
		return false;

	}

	if (evaluator.eval_type != Type::Decimal && evaluator.eval_type != Type::Integer) {

		error.error = true;
		error.info = "Precedence expression must be numeric:";
		error.sources.push_back(exp.cast<Printable>());

		finished = true;
		return false;

	}
	
	if (exp_count == 0) sym_precedence.p0 = evaluator.getNumericValue();
	else if (exp_count == 1) sym_precedence.p1 = evaluator.getNumericValue();
	else sym_precedence.p2 = evaluator.getNumericValue();

	exp_count++;
	return true;

}

AST::SymbolTableLinker AST::Builder_Body_Precedence::getSymbols() { return symbols.attachTree(variable_search_tree.cast<SymbolSearchTreeBase>()); }