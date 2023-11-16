#include "./Expression.hpp"
#include "./../../../Builder/implementations/Builder_Body_Structure.hpp"
#include "./../../../Pattern/implementations/Declaration.hpp"
#include "./../../../utility_functions.hpp"
#include "./../../../Symbol/SymbolSearchTree.hpp"

AST::Expression::Expression() : Pattern(PatternID::Expression) { }

string AST::Expression::getInfo(int alignment) { return "\n" + node->print(alignment + 1, false); }

Printable* AST::Expression::singleLine() { return this; }

unsigned long long AST::Expression::total_successors_generated = 0;

#define node_cache (*cache)

AST::Scanner_Expression::Scanner_Expression(Code::Loader& loader, int start, int end, int cur_best_start, SymbolTableLinker symbols)
	: Scanner(loader, start, end, cur_best_start), loader(loader), start(start), end(end), symbols(symbols) {

	if (cache) getWinner(node_cache[start]);
	if (result != nullptr) return;

	ExpressionStack expression_stack;
	ValidatorStack validator_stack;

	generateSuccessors(expression_stack, validator_stack, start);
	getWinner(candidates);

}

AST::ptr_Symbol AST::Scanner_Expression::banned_symbol;

void AST::Scanner_Expression::cleanup() { banned_symbol = nullptr; }

void AST::Scanner_Expression::clearCache() {

	for (auto& c : *cache) c.clear();

}

AST::NodeCache* AST::Scanner_Expression::cache = nullptr;

bool AST::Scanner_Expression::requirements[(int)AST::NodeID::_size];

AST::NodeID AST::Scanner_Expression::_singular_requirement = NodeID::None;

int AST::Scanner_Expression::_num_reqs;

void AST::Scanner_Expression::generateSuccessors(ExpressionStack& expression_stack, ValidatorStack& validator_stack, int start) {

	NodeList successors;

	generateExpressionSuccessors(expression_stack, validator_stack, start, successors);

	generateLiteralSuccessors(expression_stack, validator_stack, start, successors);

	generateVariableSuccessors(expression_stack, validator_stack, start, successors);

	generateFillerSuccessors(expression_stack, validator_stack, start, successors);
	
	generateIntermediateFillerSuccessors(expression_stack, validator_stack, start, successors);

	generateStructureMemberSuccessors(expression_stack, validator_stack, start, successors);

	generateLabelSuccessors(expression_stack, validator_stack, start, successors);

	Expression::total_successors_generated += successors.size();
	
	if (successors.size() == 0) {
		
		terminateBranch(expression_stack, validator_stack);

		return;

	}
	
	for (auto succ : successors) {
		
		if (successors.size() == 1) {

			expression_stack.push(succ);

			if (validateExpression(expression_stack, validator_stack))
				generateSuccessors(expression_stack, validator_stack, succ->end);

			else terminateBranch(expression_stack, validator_stack);

			return;

		}

		auto new_expression_stack = expression_stack;
		auto new_validator_stack = validator_stack;

		new_expression_stack.push(succ);

		if (validateExpression(new_expression_stack, new_validator_stack))
			generateSuccessors(new_expression_stack, new_validator_stack, succ->end);

		else terminateBranch(new_expression_stack, new_validator_stack);

	}

}

void AST::Scanner_Expression::generateExpressionSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Expression)) return;

	const char* kw = "(";
	int par_start = start;

	if (matchString_(loader, kw, par_start) >= 0) {

		Scanner_Expression exp(loader, par_start + 1, end, 0, symbols);

		if (exp.result != nullptr) {

			auto exp_result = exp.result.cast<Expression>();

			kw = ")";
			int par_end = exp_result->node->end;

			if (matchString_(loader, kw, par_end) >= 0) successors.push_back(new ExpressionNode(exp_result, start, par_end + 1));

		}

	}

}

void AST::Scanner_Expression::generateLiteralSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Literal)) return;

	Scanner_Literal scanner_literal(loader, start, end);

	if (scanner_literal.result != nullptr) {

		auto literal_match = scanner_literal.result.cast<Literal>();
		successors.push_back(new LiteralNode(literal_match));

	}

}

inline const char* _genSearchKW(Code::Loader& loader, int start) {

	static char kw[AST_SEARCH_TREE_MAX_DEPTH];

	int i;
	int lsize = loader.size();

	for (i = 0; i < AST_SEARCH_TREE_MAX_DEPTH; i++) {

		if (start == lsize) break;

		while (loader(start).is_whitespace) {

			start++;
			if (start == lsize) break;

		}

		if (start == lsize) break;

		kw[i] = loader[start];
		start++;

	}

	if (i < AST_SEARCH_TREE_MAX_DEPTH) kw[i] = '}';
	
	return (const char*)kw;

}

void AST::Scanner_Expression::generateVariableSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Variable)) return;
	if (symbols.variable_search_tree == nullptr) return;

	auto search_tree = symbols.variable_search_tree.cast<VariableSearchTree>();
	auto it = search_tree->iterate(_genSearchKW(loader, start));

	while (!it.finished()) {

		int match = matchString(loader, it.cur_bundle->test_kw.c_str(), start);

		if (match >= 0) {

			auto sym = it.cur_bundle->syms[0];
			if (match >= 0) successors.push_back(new VariableNode(sym, start, match));

		}

		it.next();
		
	}

	auto it_local = search_tree->local_symbols.iterate(SymbolID::Variable);

	while (!it_local.finished()) {

		auto sym = it_local();
		auto& var_name = sym->operator[](0).name;

		int match = matchString(loader, var_name, start);
		if (match >= 0) successors.push_back(new VariableNode(sym, start, match));

		it_local = it_local.next();

	}

}

void AST::Scanner_Expression::generateFillerSuccessors(ExpressionStack& expression_stack, ValidatorStack& validator_stack, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Filler)) return;
	if (symbols.pattern_match_search_tree == nullptr) return;
	
	auto search_tree = symbols.pattern_match_search_tree.cast<PatternMatchSearchTree>();
	auto it = search_tree->iterate(_genSearchKW(loader, start));

	while (!it.finished()) {

		int match = matchString(loader, it.cur_bundle->test_kw.c_str(), start);

		if (match >= 0) {

			auto sym = it.cur_bundle->firstSym();
			int sym_idx = sym->filler_indices[0];

			successors.push_back(new FillerNode(it.cur_bundle, sym_idx, start, match));

		}

		it.next();
		
	}

}

void AST::Scanner_Expression::generateIntermediateFillerSuccessors(ExpressionStack& expression_stack, ValidatorStack& validator_stack, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Filler)) return;

	for (auto& val : validator_stack()) {

		if (val.next_filler < 0) continue;
		if (val.distance != 1) continue;

		auto bundle = val.bundle;
		auto example_sym = bundle->syms[0].cast<HeaderSymbol>();

		int sym_idx = example_sym->filler_indices[val.next_filler];
		auto& var_name = example_sym->operator[](sym_idx).name;

		int var_match = matchString(loader, var_name, start);
		if (var_match >= 0) successors.push_back(new FillerNode(bundle, sym_idx, start, var_match));
		
	}

}

void AST::Scanner_Expression::generateStructureMemberSuccessors(ExpressionStack& expression_stack, ValidatorStack& validator_stack, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::StructureMember)) return;

	if (expression_stack.size() == 0) return;

	const char* kw = ".";
	int new_start = matchString(loader, kw, start);

	if (new_start < 0) return;

	auto node = expression_stack.top();

	if (node->ID == NodeID::Filler) {

		if (validator_stack.size() == 0) return;

		auto& val = validator_stack.top();
		if (val.distance <= 0 && val.next_filler < 0) successors.push_back(new EmptyNode(start));

		return;

	}

	auto it = symbols.iterate(SymbolID::Structure);
	int orig_num_succs = successors.size();

	while (!it.finished()) {

		auto& structure_varnames = ((Builder*)(it().cast<HeaderSymbol>()->body))->patterns;

		for (auto& varname : structure_varnames) {

			if (varname->ID != PatternID::Declaration) continue;
			auto decl = varname.cast<Declaration>();

			auto& name = decl->sym->operator[](0).name;
			int varname_match = matchString(loader, name, new_start);

			if (varname_match >= 0) {

				bool is_repeat = false;
				for (int i = orig_num_succs; i < successors.size(); i++) if (successors[i].cast<StructureMemberKWNode>()->kw == name) { is_repeat = true; break; }

				if (!is_repeat) successors.push_back(new StructureMemberKWNode(name, start, varname_match));

			}

		}

		it = it.next();

	}

}

void AST::Scanner_Expression::generateLabelSuccessors(ExpressionStack& expression_stack, ValidatorStack& validator_stack, int start, NodeList& successors) {

	if (!isPossibleSuccessor(NodeID::Label)) return;

	static const SymbolID _fs[] = { SymbolID::Function, SymbolID::Structure };

	for (int i = 0; i < 2; i++) {

		auto type = _fs[i];
		auto it = symbols.iterate(type);

		while (!it.finished()) {

			auto sym = it().cast<HeaderSymbol>();

			if (sym->label.size() > 0) {

				int label_match = matchString(loader, sym->label, start);
				if (label_match >= 0) successors.push_back(new LabelNode(sym, start, label_match));
				
			}

			it = it.next();

		}

	}

}

bool AST::Scanner_Expression::validateExpression(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	if (expression_stack.size() == 0) return true;
	
	auto node = expression_stack.top();

	if (node->ID == NodeID::StructureMemberKW) {

		handleStructureMembers(expression_stack);
		return true;

	}

	if (node->ID == NodeID::None) return handleFlushes(expression_stack, validator_stack);

	updateValidators(expression_stack, validator_stack);

	while (patternMatchExists(expression_stack, validator_stack))
		if (!collapsePatternMatch(expression_stack, validator_stack)) return false;
	
	return createValidator(expression_stack, validator_stack);

}

void AST::Scanner_Expression::handleStructureMembers(ExpressionStack& expression_stack) {

	auto top = expression_stack.top();

	auto member_kw = top.cast<StructureMemberKWNode>();
	expression_stack.pop();

	top = expression_stack.top();

	if (top->ID == NodeID::StructureMember) { top.cast<StructureMemberNode>()->addMember(member_kw->kw, member_kw->end); return; }

	ptr_StructureMemberNode new_top = new StructureMemberNode(top);
	new_top->addMember(member_kw->kw, member_kw->end);

	expression_stack.pop();
	expression_stack.push(new_top.cast<Node>());

}

bool AST::Scanner_Expression::handleFlushes(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	expression_stack.pop();
	return collapsePatternMatch(expression_stack, validator_stack);

}

void AST::Scanner_Expression::updateValidators(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	auto node = expression_stack.size() == 0 ? nullptr : expression_stack.top()();
	auto filler_node = !node ? nullptr : node->ID == NodeID::Filler ? (FillerNode*)node : nullptr;

	Validator* filler_collision = nullptr;

	for (auto& v : validator_stack()) {

		v.distance--;

		if (filler_node) 
			if (v.distance == 0 && v.next_filler >= 0) 
				if (filler_node->sym_idx == v.bundle->firstSym()->filler_indices[v.next_filler]) 
					filler_collision = &v;

	}

	if (filler_collision) {

		auto collision_sym = filler_collision->bundle->firstSym();
		int new_distance = collision_sym->distanceToNextFiller(filler_collision->next_filler);

		filler_collision->next_filler++;
		if (filler_collision->next_filler == collision_sym->num_fillers) filler_collision->next_filler = -1;

		for (auto& v : validator_stack()) v.distance += new_distance;

	}

}

bool AST::Scanner_Expression::patternMatchExists(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	if (validator_stack.size() == 0) return false;

	auto val = validator_stack.top();
	auto val_sym = val.bundle->firstSym();

	if (val.next_filler >= 0) return false;
	if (val.distance > 0) return false;

	auto node = expression_stack.top();
	bool pattern_matched = false;

	if (!val_sym->operator[](val_sym->size() - 1).is_argument) pattern_matched = true;

	else if (node->isArgument()) pattern_matched = false;

	else {

		auto filler_node = node.cast<FillerNode>();

		if (filler_node->bundle == val.bundle || val.distance < 0) {

			auto next_pattern_sym = filler_node->sym.cast<HeaderSymbol>();

			if (filler_node->sym_idx == 0) pattern_matched = false;
			else if (!next_pattern_sym->operator[](0).is_argument) pattern_matched = true;
			else if (next_pattern_sym->precedence_lh < val_sym->precedence_rh) pattern_matched = true;

		}

	}

	return pattern_matched;

}

bool AST::Scanner_Expression::collapsePatternMatch(ExpressionStack& expression_stack, ValidatorStack& validator_stack, Validator* v) {
	
	auto val = v ? *v : validator_stack.top();
	auto val_sym = val.bundle->firstSym();
	int match_end_index = val.index + val_sym->size();

	for (int i = val.index; i < match_end_index; i++) {

		bool is_arg_exp = expression_stack[i]->isArgument();
		bool is_arg_sym = val_sym->operator[](i - val.index).is_argument;

		if (is_arg_exp != is_arg_sym) return false;

	}

	int overflow = expression_stack.size() - match_end_index;
	
	ExpressionStack temp;
	for (int i = 0; i < overflow; i++) { temp.push(expression_stack.top()); expression_stack.pop(); }
	
	ptr_PatternMatchNode match_node = new PatternMatchNode(val.bundle);

	for (int i = val.index; i < match_end_index; i++) match_node->addComponent(expression_stack[i]);
	for (int i = 0; i < val_sym->size(); i++) expression_stack.pop();
	
	expression_stack.push(match_node.cast<Node>());
	for (int i = 0; i < overflow; i++) { expression_stack.push(temp.top()); temp.pop(); }
	
	validator_stack.pop();
	return true;

}

bool AST::Scanner_Expression::createValidator(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	if (expression_stack.size() == 0) return true;

	auto node = expression_stack.top();

	if (node->isArgument()) return true;

	auto filler_node = node.cast<FillerNode>();

	auto bundle = filler_node->bundle;
	auto sym = bundle->firstSym();
	
	if (filler_node->sym_idx != sym->filler_indices[0]) return true;
	if (getArgumentStreakLength(expression_stack, 1) < filler_node->sym_idx) return false;

	int index = expression_stack.size() - 1 - filler_node->sym_idx;
	int distance = sym->distanceToNextFiller(0);
	int next_filler = sym->num_fillers == 1 ? -1 : 1;

	for (auto& v : validator_stack()) v.distance += distance + filler_node->sym_idx;

	Validator new_validator{ bundle, index, distance, next_filler };
	validator_stack.push(new_validator);

	return true;

}

void AST::Scanner_Expression::terminateBranch(ExpressionStack& expression_stack, ValidatorStack& validator_stack) {

	for (int i = validator_stack.size() - 1; i >= 0; i--) {

		auto& val = validator_stack[i];
		if (val.next_filler < 0 && val.distance <= 0) if (!collapsePatternMatch(expression_stack, validator_stack, &val)) break;

	}

	if (expression_stack.size() == 0) return;
	if (expression_stack[0]->isArgument()) candidates.push_back(expression_stack[0]);

	for (int i = 1; i < expression_stack.size(); i++) {

		auto node = expression_stack[i];
		if (!node->isArgument()) return;

		addToCache(node);

	}
	
}

void AST::Scanner_Expression::getWinner(NodeList& candidates) {

	if (candidates.size() == 0) return;

	ptr_Node winner;
	ptr_Node conflict;

	for (auto c : candidates) {

		if (!requirements[0] && !requirements[(int)c->ID]) continue;

		if (c->start > start) continue;

		if (!c->isArgument()) continue;

		if (winner == nullptr) { winner = c; continue; }

		if (c->end < winner->end) continue;

		if (c->end > winner->end) { winner = c; conflict = nullptr; continue; }

		if (!Node::equals(winner, c)) conflict = c;

	}

	if (winner == nullptr) return;

	ptr_Expression expression = new Expression();

	if (conflict != nullptr) {

		expression->error.error = true;
		expression->error.info = "Ambiguous expression has two equally valid interpretations:";

		string winner_str = "First interpretation " + getPositionString(loader, winner->start, winner->end) + ":\n" + winner->print(2, false);
		string conflict_str = "Second interpretation " + getPositionString(loader, conflict->start, conflict->end) + ":\n" + conflict->print(2, false);

		expression->error.sources.push_back(new PrintableString(winner_str));
		expression->error.sources.push_back(new PrintableString(conflict_str));

	}
	else if (winner->contains(banned_symbol)) {

		expression->error.error = true;
		expression->error.info = "Variable declaration at " + getPositionString(loader, winner->start, winner->end) + " contains a reference to itself in expression:";
		expression->error.sources.push_back(winner.cast<Printable>());

	}
	else {

		auto err = winner->getContainedError();
		if (err) expression->error = *err;

		else {

			auto bundle = winner->findPrecedenceConflicts();

			if (bundle) {

				CompileError error;

				error.error = true;
				error.info = "Two symbols with the same signature have conflicting precedences.";

				auto sym1 = (HeaderSymbol*)bundle->syms[bundle->conflict_first]();
				auto sym2 = (HeaderSymbol*)bundle->syms[bundle->conflict_second]();

				auto header1 = (Builder*)sym1->header;
				auto header2 = (Builder*)sym2->header;

				header1->single_line = true;
				header2->single_line = true;

				auto problem_exp1 = 
					header1
						? new PrintableString("First symbol defined at:\n" + header1->print(2, false))
						: new PrintableString(string("First symbol (built-in pattern): ") + sym1->getSignature().c_str());
				
				auto problem_exp2 = 
					header2
						? new PrintableString("Second symbol defined at:\n" + header2->print(2, false))
						: new PrintableString(string("Second symbol (built-in pattern): ") + sym2->getSignature().c_str());

				error.sources.push_back(problem_exp1);
				error.sources.push_back(problem_exp2);

				expression->error = error;

			}

		}

	}

	expression->node = winner;
	
	int cur_end = winner->end;
	while (loader(cur_end).is_whitespace && cur_end < loader.size()) cur_end++;

	if (cur_end == loader.size()) { cur_end = winner->end; } 
	else if (loader[cur_end] == ',' || loader[cur_end] == ':') cur_end++; 
	else cur_end = winner->end;
	
	winner->end = cur_end;
	winner->updateReferenceCounts();

	result = expression.cast<Pattern>();

	result->start = loader(winner->start);
	result->end = loader(winner->end);

}

int AST::Scanner_Expression::getArgumentStreakLength(ExpressionStack& expression_stack, int offset) {

	int start_index = expression_stack.size() - 1 - offset;
	int streak_length = 0;

	while (true) {

		if (start_index < 0) break;
		if (!expression_stack[start_index]->isArgument()) break;

		start_index--;
		streak_length++;

	}

	return streak_length;

}

bool AST::Scanner_Expression::isPossibleSuccessor(NodeID ID) {

	if (_singular_requirement != NodeID::None) return Node::necessitates(_singular_requirement, ID);

	for (int i = 0; i < (int)NodeID::_size; i++) if (requirements[i]) if (Node::necessitates((NodeID)i, ID)) return true;
	return false;

}

void AST::Scanner_Expression::_setRequirements(NodeID ID) { requirements[(int)ID] = true; _singular_requirement = _num_reqs == 0 ? ID : NodeID::None; _num_reqs++; }

void AST::Scanner_Expression::addToCache(ptr_Node node) { if (cache) addToCache(node, node_cache[node->start]); }

void AST::Scanner_Expression::addToCache(ptr_Node node, NodeList& list) {

	for (auto& collision : list) if (Node::equals(node, collision)) return;
	list.push_back(node);

}