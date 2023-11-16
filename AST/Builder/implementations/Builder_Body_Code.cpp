#include "./Builder_Body_Code.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"
#include "./../../Pattern/implementations/Declaration.hpp"
#include "./../../Pattern/implementations/Assignment.hpp"
#include "./../../Pattern/implementations/Keyword.hpp"
#include "./../../Pattern/implementations/ForLoopHeader.hpp"
#include "./../../Pattern/implementations/Body.hpp"
#include "./../../Symbol/SymbolSearchTree.hpp"

AST::Builder_Body_Code::Builder_Body_Code(BuilderID ID, Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols)
	: Builder(ID, loader, start, end) {

	pattern_match_search_tree = symbols.pattern_match_search_tree.cast<PatternMatchSearchTree>();
	variable_search_tree = symbols.variable_search_tree.cast<VariableSearchTree>();
	vst_initial_table = variable_search_tree->local_symbols;
	
	this->main_scope = scope;
	this->symbols = symbols;

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::PatternMatch, NodeID::Variable, NodeID::StructureMember);

}

bool AST::Builder_Body_Code::is_loop_body = false;

bool AST::Builder_Body_Code::isElseKW(PatternID ID) { return ID == PatternID::Keyword_else || ID == PatternID::Keyword_else_comma || ID == PatternID::Keyword_else_colon; }

string AST::Builder_Body_Code::printChildren(int alignment) { return code_pieces.toString(alignment); }

#define _finish_piece() incrementElseDist(); allow_code_body = false; is_loop_body = next_is_loop_body

bool AST::Builder_Body_Code::processPattern(ptr_Pattern p) {

	allow_expression = true;

	if (new_piece) {

		is_control_flow = false;
		next_is_loop_body = is_within_loop_body;

		current_for_table = (allow_code_body || isElseKW(p->ID)) ? current_for_table : SymbolTableLinker();

		if (p->ID == PatternID::Expression) {

			auto exp = p.cast<Expression>();
			auto node = exp->node;
			
			require_assignment = node->ID == NodeID::Variable || node->ID == NodeID::StructureMember;

			if (require_assignment) if (!verifyAssignment(node->end)) {

				allow_expression = false;
				require_assignment = false;

				return false;

			}

			if (require_assignment) Scanner_Assignment::expression = exp;

			new_piece = !require_assignment;

			if (new_piece) { 
				
				_finish_piece();
				
			}

			return true;

		}

		if (p->ID == PatternID::Declaration) {

			auto new_var = p.cast<Declaration>()->sym;
			new_var->declaration = p();

			table->add(new_var);

			new_piece = false;

			Scanner_Expression::banned_symbol = new_var;
			Scanner_Expression::setRequirements(NodeID::None);

			return true;

		}

		if (p->ID == PatternID::Keyword_return) {

			new_piece = false;
			
			Scanner_Expression::banned_symbol = nullptr;
			Scanner_Expression::setRequirements(NodeID::None);

			return true;

		}

		if (p->ID == PatternID::Body_Function) { 
			
			

			Scanner_Expression::banned_symbol = nullptr;
			Scanner_Expression::setRequirements(NodeID::PatternMatch, NodeID::Variable, NodeID::StructureMember);

			_finish_piece();

			return true;
		
		}

		if (p->ID == PatternID::Keyword_continue || p->ID == PatternID::Keyword_break) {
			
			if (!is_loop_body) {

				error.error = true;
				error.info = "Loop execution modifier keywords are not allowed outside of loops. Source keyword:";
				error.sources.push_back(p.cast<Printable>());

				finished = true;
				return false;

			}

			_finish_piece();

			return true;

		}

		else_dist = 0;

		if (isElseKW(p->ID)) { 
			
			elses_allowed--;

			current_for_table = last_if_for_table.top();
			last_if_for_table.pop();

			allow_code_body = true;
			is_control_flow = true;

			return true;
		
		}

		if (p->ID == PatternID::Keyword_if) { elses_allowed++; last_if_for_table.push(current_for_table); }

		if (p->ID == PatternID::Keyword_for || p->ID == PatternID::Keyword_while) next_is_loop_body = true;
		
		new_piece = false;
		is_control_flow = true;

		Scanner_Expression::banned_symbol = nullptr;
		Scanner_Expression::setRequirements(NodeID::None);

		return true;

	}

	if (p->ID == PatternID::Expression)

		if (patterns.size() > 0 && patterns[patterns.size() - 1]->ID == PatternID::Declaration) {
			
			auto decl = patterns[patterns.size() - 1].cast<Declaration>();
			decl->sym->definition = p();

			Scanner_Expression::clearCache();

		}

	if (p->ID == PatternID::Assignment) {

		patterns.resize(patterns.size() - 1);

		Scanner_Expression::banned_symbol = nullptr;
		Scanner_Expression::setRequirements(NodeID::None);

		return true;

	}

	if (p->ID == PatternID::ForLoopHeader) current_for_table = current_for_table.linkWith(p.cast<ForLoopHeader>()->table);

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::PatternMatch, NodeID::Variable, NodeID::StructureMember);

	new_piece = true;
	allow_code_body = is_control_flow;
	is_loop_body = next_is_loop_body;

	if (!is_control_flow) incrementElseDist();

	return true;

}

void AST::Builder_Body_Code::processCodePieces() { for (auto p : patterns) code_pieces.scanNextPattern(p); }

AST::SymbolTableLinker AST::Builder_Body_Code::getSymbols() { 

	auto local_symbols = (new_piece && !allow_code_body) ? SymbolTableLinker{ table } : SymbolTableLinker{ table }.linkWith(current_for_table);
	auto syms = symbols.linkWith(local_symbols);

	if (variable_search_tree != nullptr)
		variable_search_tree->local_symbols = vst_initial_table.linkWith(local_symbols);

	return syms.attachTree(pattern_match_search_tree.cast<SymbolSearchTreeBase>()).attachTree(variable_search_tree.cast<SymbolSearchTreeBase>());

}

bool AST::Builder_Body_Code::verifyAssignment(int exp_end) {

	while (loader(exp_end).is_whitespace) exp_end++;
	return loader[exp_end] == '=';

}

void AST::Builder_Body_Code::incrementElseDist() {

	if (elses_allowed == 0) return;

	else_dist++;
	if (else_dist > 1) { elses_allowed = 0; else_dist = 0; last_if_for_table.clear(); }

}