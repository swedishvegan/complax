#include "./Builder_Body_Restrictions.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/FreePattern.hpp"
#include "./../../Pattern/implementations/Assignment.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"

AST::Builder_Body_Restrictions::Builder_Body_Restrictions(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols)
	: Builder(BuilderID::Builder_Body_Restrictions, loader, start, end) {

	this->main_scope = scope;
	this->symbols = symbols.linkWith(table);
	this->pattern_match_search_tree = symbols.pattern_match_search_tree.cast<PatternMatchSearchTree>();
	this->variable_search_tree = symbols.variable_search_tree.cast<VariableSearchTree>();
	
	if (!findHeaderTable()) return;

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::None);

	build();
	Scanner_Expression::cleanup();

	if (error.error) return;

	if (patterns.size() > 0 && !expecting_var) {

		error.error = true;
		error.info = "Expected type after header variable:";
		error.sources.push_back(patterns[patterns.size() - 1].cast<Printable>());

	}

}

string AST::Builder_Body_Restrictions::printChildren(int alignment) { 

	if (restrictions.empty()) return "Restrictions (empty)";

	string s;

	for (auto& r : restrictions) {

		s += indent(alignment) + "Variable:       " + r.first->getSignature().c_str() + "\n";
		for (auto exp : r.second) s += exp->node->print(alignment + 1);

	}

	return s.substr(0, s.size() - 1);

}

AST_DECL_SUCCESSORS(Builder_Body_Restrictions) {

	AST_SUCC_ALLOW(Expression);
	AST_SUCC_ALLOW(Done);

}

bool AST::Builder_Body_Restrictions::processPattern(ptr_Pattern p) {
	
	auto exp = p.cast<Expression>();
	auto node = exp->node;

	ptr_Symbol var;
	
	if (node->ID == NodeID::Variable) {

		auto var_candidate = node.cast<VariableNode>()->sym;
		if (header_table->conflictsWith(*var_candidate)) var = var_candidate;

	}

	if (expecting_var) {

		if (var == nullptr && !expecting_type) {

			error.error = true;
			error.info = "Expected header variable at " + loader(start).toString() + ".";

			finished = true;
			return false;

		}

		if (var != nullptr) {

			expecting_var = false;
			expecting_type = true;

			cur_var = var;

			return true;

		}

	}

	if (expecting_type) {

		if (var != nullptr && !expecting_var) {

			error.error = true;
			error.info = "Expected type after header variable:";
			error.sources.push_back(patterns[patterns.size() - 1].cast<Printable>());

			finished = true;
			return false;

		}

		if (var == nullptr) {

			restrictions[cur_var].push_back(exp);

			expecting_var = true;
			expecting_type = true;

		}
		
	}

	return true;

}

AST::SymbolTableLinker AST::Builder_Body_Restrictions::getSymbols() { return symbols.attachTree(pattern_match_search_tree.cast<SymbolSearchTreeBase>()).attachTree(variable_search_tree.cast<SymbolSearchTreeBase>()); }

bool AST::Builder_Body_Restrictions::findHeaderTable() {

	auto linker = &symbols;

	while (linker) {

		if (linker->table != nullptr) if (linker->table->is_header) { header_table = linker->table; return true; }
		linker = linker->next();

	}

	return false;

}