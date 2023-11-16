#ifndef GLOBALSCOPEBUILDER_HPP
#define GLOBALSCOPEBUILDER_HPP

#include "./../Builder.hpp"
#include "./Builder_Header.hpp"

#define SEARCH_TREE_DEPTH 6

namespace AST {

	struct GlobalScopeBuilder : public Builder {

		GlobalScopeBuilder(void* codebase, Code::Loader& loader);

		static Pattern* main_function; // Type evaluation step begins here at the main function

	protected:

		void* codebase; // Code::Bundle pointer, must be void* to avoid mutual inclusions

		bool allow_impinc = true;
		bool allow_body = false;
		bool allow_restrictions = false;
		bool allow_label = false;
		bool allow_returns_kw = false;
		bool allow_returns_exp = false;
		int precedence_count = 0;

		bool last_pattern_was_declaration = false;

		Builder_Header* relevant_header = nullptr;
		SymbolID relevant_header_type = SymbolID::None;

		SymbolTableLinker imports;

		void generateSuccessors(PatternID cur_ID);

		bool processPattern(ptr_Pattern);

		SymbolTableLinker getSymbols();

	};

}

#endif