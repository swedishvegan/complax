#include "./Builder.hpp"
#include "./../utility_functions.hpp"

AST::Builder::Builder(BuilderID ID, Code::Loader& loader, int start, int end) 
	: ID(ID), 
	loader(loader), 
	start(start), end(end), 
	table(new SymbolTable((
		ID == BuilderID::GlobalScopeBuilder || 
		ID == BuilderID::Builder_Body_Function || 
		ID == BuilderID::Builder_Body_Structure
	) ? end - start : 1)) { }

void AST::Builder::build() {
	
	next_start = start;
	auto cur_ID = PatternID::None;

	while (true) {
		
		getNextPattern(cur_ID);
		
		if (finished) break;
		if (patterns.size() > 0) cur_ID = patterns[patterns.size() - 1]->ID;

	}

	checkForFinalErrors();

}

AST::Builder::~Builder() { }

string AST::Builder::toString(int alignment) {

	bool multi_line = !single_line && patterns.size() > 0;

	auto po_str = getPositionString(loader, start, end);
	if (multi_line) po_str += ":";

	string s = pad(getBuilderIDString(ID)) + " " +  pad(po_str, AST_POSITION_PAD_LENGTH);
	if (!multi_line) return s;

	s += "\n" + printChildren(alignment + 1);

	return s;

}

string AST::Builder::printChildren(int alignment) {
	
	string s;
	if (patterns.size() == 0) return s;

	for (int i = 0; i < patterns.size() - 1; i++) s += patterns[i]->print(alignment);
	s += patterns[patterns.size() - 1]->print(alignment, false);

	return s;

}

void AST::Builder::checkForFinalErrors() {

	if (error.error) return;

	if (!isRangeEmpty(loader, next_start, end)) {

		error.error = true;

		if (patterns.size() > 0) {

			error.info = "Non-whitespace character(s) in builder " + getPositionString(loader, start, end) + " following final pattern:";

			auto final_pattern = patterns[patterns.size() - 1];
			final_pattern->single_line = true;

			error.sources.push_back(final_pattern.cast<Printable>());

		}

		else error.info = "Non-whitespace character(s) present in empty builder " + getPositionString(loader, start, end) + ".";

		return;

	}

	if (ID != BuilderID::Builder_Body_Function && ID != BuilderID::Builder_Body_Structure) return;

	auto syms = SymbolTableLinker{ table };
	auto it = syms.iterate(SymbolID::Variable);

	while (!it.finished()) {

		if (it()->num_references == 0) {

			error.error = true;
			error.info = "Variable declared but never used:";
			error.sources.push_back(new PrintableString(((Pattern*)it()->declaration)->print(0, false)));

			return;

		}

		it = it.next();

	}

}

void AST::Builder::allowSuccessor(PatternID cur_ID) {

	for (int i = 0; i < AST_BUILDER_MAX_SUCCESSORS; i++) {

		if (successor_IDs[i] == cur_ID) return;
		if (successor_IDs[i] == PatternID::None) { successor_IDs[i] = cur_ID; return; }

	}

}

void AST::Builder::getNextPattern(PatternID cur_ID) {
	
	for (int i = 0; i < AST_BUILDER_MAX_SUCCESSORS; i++) successor_IDs[i] = PatternID::None;
	this->generateSuccessors(cur_ID);
	
	if (successor_IDs[0] == PatternID::None) { finished = true; return; }

	ptr_Pattern best = nullptr;
	ptr_Pattern conflict = nullptr;

	int cur_best_start = end;

	for (int i = 0; i < AST_BUILDER_MAX_SUCCESSORS; i++) {

		if (successor_IDs[i] == PatternID::None) break;
		
		auto contender = trySuccessor(successor_IDs[i], cur_best_start);
		if (contender == nullptr) continue;

		if (best == nullptr) { best = contender; continue; }

		int comp = comparePatterns(best, contender);

		if (comp == 0) conflict = contender;
		else if (comp == 1) { best = contender; conflict = nullptr; }

		if (best != nullptr) cur_best_start = best->start.index;

	}

	if (best == nullptr) {

		bool could_be_done = false;

		for (int i = 0; i < AST_BUILDER_MAX_SUCCESSORS; i++) {

			if (successor_IDs[i] == PatternID::None) break;
			if (successor_IDs[i] == PatternID::Done) { could_be_done = true; break; }

		}
		
		if (!could_be_done) {

			error.error = true;
			error.info = "Expected successor at " + loader(next_start).toString() + " but none was found. Expecting one of the following:";

			for (int i = 0; i < AST_BUILDER_MAX_SUCCESSORS; i++) {

				if (successor_IDs[i] == PatternID::None) break;
				error.sources.push_back(new PrintableString(getPatternIDString(successor_IDs[i])));

			}

		}

		finished = true;
		return;

	}
	
	if (conflict != nullptr) {

		error.error = true;
		error.info = "Pattern conflict.";

		best->single_line = true;
		conflict->single_line = true;

		error.sources.push_back(best.cast<Printable>());
		error.sources.push_back(conflict.cast<Printable>());

		finished = true;
		return;

	}
	
	if (best->error.error) {
		
		error = best->error;

		finished = true;
		return;

	}
	
	if (this->processPattern(best)) {

		if (finished) return;

		patterns.push_back(best);
		next_start = best->end.index;

	}

}

int AST::Builder::comparePatterns(ptr_Pattern LH, ptr_Pattern RH) {

	if (LH->precedence > RH->precedence) return -1;
	if (RH->precedence > LH->precedence) return 1;
	
	if (LH->start < RH->start) return -1;
	if (RH->start < LH->start) return 1;

	if (LH->end > RH->end) return -1;
	if (RH->end > LH->end) return 1;

	return 0;

}

#define AST_BLDR_ADD_SCANNER_(PatternType, const_args) \
\
if (cur_ID == PatternID::PatternType) { \
	\
	Scanner_##PatternType scanner(const_args); \
	if (scanner.result != nullptr) return scanner.result; \
	\
}

#define AST_BLDR_ADD_SCANNER(PatternType) AST_BLDR_ADD_SCANNER_(PatternType, AST_VA_ARGS(loader, next_start, end, cur_best_start))

#define AST_BLDR_ARGS AST_VA_ARGS(loader, next_start, end, cur_best_start, main_scope + 1)
#define AST_ARGS_W_SYMS AST_VA_ARGS(loader, next_start, end, cur_best_start, getSymbols())
#define AST_BLDR_ARGS_W_SYMS AST_VA_ARGS(loader, next_start, end, cur_best_start, main_scope + 1, getSymbols())

#include "./../Pattern/implementations/Keyword.hpp"
#include "./../Pattern/implementations/Literal.hpp"
#include "./../Pattern/implementations/FreePattern.hpp"
#include "./../Pattern/implementations/Header.hpp"
#include "./../Pattern/implementations/Body.hpp"
#include "./../Pattern/implementations/Expression/Expression.hpp"
#include "./../Pattern/implementations/Declaration.hpp"
#include "./../Pattern/implementations/Assignment.hpp"
#include "./../Pattern/implementations/ForLoopHeader.hpp"

AST::ptr_Pattern AST::Builder::trySuccessor(PatternID cur_ID, int cur_best_start) {
	
	this->cur_ID = cur_ID;

	// Keyword implementations

	AST_BLDR_ADD_SCANNER(Keyword_startprogram);

	AST_BLDR_ADD_SCANNER(Keyword_function);
	AST_BLDR_ADD_SCANNER(Keyword_structure);

	AST_BLDR_ADD_SCANNER(Keyword_wrest);
	AST_BLDR_ADD_SCANNER(Keyword_wprec);
	AST_BLDR_ADD_SCANNER(Keyword_precarrow);
	AST_BLDR_ADD_SCANNER(Keyword_wlabl);

	AST_BLDR_ADD_SCANNER(Keyword_import);
	AST_BLDR_ADD_SCANNER(Keyword_include);

	AST_BLDR_ADD_SCANNER(Keyword_for);
	AST_BLDR_ADD_SCANNER(Keyword_while);

	AST_BLDR_ADD_SCANNER(Keyword_if);
	AST_BLDR_ADD_SCANNER(Keyword_else);
	AST_BLDR_ADD_SCANNER(Keyword_else_comma);
	AST_BLDR_ADD_SCANNER(Keyword_else_colon);

	AST_BLDR_ADD_SCANNER(Keyword_return);
	AST_BLDR_ADD_SCANNER(Keyword_returns);

	AST_BLDR_ADD_SCANNER(Keyword_continue);
	AST_BLDR_ADD_SCANNER(Keyword_break);

	AST_BLDR_ADD_SCANNER(Keyword_let);

	// Literals

	AST_BLDR_ADD_SCANNER(Literal);

	// FreePattern implementations

	AST_BLDR_ADD_SCANNER(Argument);
	AST_BLDR_ADD_SCANNER(Filler);
	AST_BLDR_ADD_SCANNER(Declaration_base);
	AST_BLDR_ADD_SCANNER(Label);

	// Header implementations
	
	AST_BLDR_ADD_SCANNER_(Header_Function, AST_BLDR_ARGS);
	AST_BLDR_ADD_SCANNER_(Header_Structure, AST_BLDR_ARGS);

	// Body implementations
	
	AST_BLDR_ADD_SCANNER_(Body_Function, AST_BLDR_ARGS_W_SYMS);
	AST_BLDR_ADD_SCANNER_(Body_Structure, AST_BLDR_ARGS_W_SYMS);
	AST_BLDR_ADD_SCANNER_(Body_Restrictions, AST_BLDR_ARGS_W_SYMS);
	AST_BLDR_ADD_SCANNER_(Body_Precedence, AST_BLDR_ARGS_W_SYMS);

	// Expressions

	AST_BLDR_ADD_SCANNER_(Expression, AST_ARGS_W_SYMS);

	// Miscellaneous

	AST_BLDR_ADD_SCANNER(Declaration);
	AST_BLDR_ADD_SCANNER(Assignment);
	AST_BLDR_ADD_SCANNER_(ForLoopHeader, AST_ARGS_W_SYMS);

	return nullptr;

}

bool AST::Builder::processPattern(ptr_Pattern) { return true; }

AST::SymbolTableLinker AST::Builder::getSymbols() { return SymbolTableLinker{ }; }