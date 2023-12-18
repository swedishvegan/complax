#ifndef KEYWORD_HPP
#define KEYWORD_HPP

#include "./../Pattern.hpp"

// Patterns and Scanners for keyword detection
//     See line 47 for Keyword implementations

namespace AST {

	struct Keyword : public Pattern {

		const char* kw;

		Keyword(PatternID, const char*);

	};

	using ptr_Keyword = ptr<Keyword>;

	template <typename Keyword_type>
	struct Scanner_Keyword : public Scanner {

		Scanner_Keyword(Code::Loader& loader, int start, int end, int cur_best_start, const char* kw) : Scanner(loader, start, end, cur_best_start) {
			
			int s = start;
			int e = matchString_(loader, kw, s);
			
			if (e < 0 || e > end || s > cur_best_start) return;
			
			ptr<Keyword_type> keyword = new Keyword_type();

			keyword->start = loader(s);
			keyword->end = loader(e);

			result = keyword.template cast<Pattern>();

		}

	};

#define AST_DECL_KEYWORD(classname) \
\
struct Keyword_##classname : public Keyword { Keyword_##classname(); }; \
struct Scanner_Keyword_##classname : public Scanner_Keyword<Keyword_##classname> { Scanner_Keyword_##classname(Code::Loader& loader, int start, int end, int cur_best_start); };

	// Keyword declarations begin here, see line 12 of Keyword.cpp for constructor definitions

	AST_DECL_KEYWORD(startprogram);

	AST_DECL_KEYWORD(function);
	AST_DECL_KEYWORD(structure);

	AST_DECL_KEYWORD(wrest);
	AST_DECL_KEYWORD(wprec);
	AST_DECL_KEYWORD(precarrow);
	AST_DECL_KEYWORD(wlabl);

	AST_DECL_KEYWORD(import);
	AST_DECL_KEYWORD(include);

	AST_DECL_KEYWORD(for);
	AST_DECL_KEYWORD(while);

	AST_DECL_KEYWORD(if);
	AST_DECL_KEYWORD(else);
	AST_DECL_KEYWORD(else_comma);
	AST_DECL_KEYWORD(else_colon);

	AST_DECL_KEYWORD(return);
	AST_DECL_KEYWORD(returns);

	AST_DECL_KEYWORD(continue);
	AST_DECL_KEYWORD(break);

	AST_DECL_KEYWORD(let);

}

#endif