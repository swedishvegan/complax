#ifndef PATTERNID_HPP
#define PATTERNID_HPP

namespace AST {

	enum class PatternID {

		None, // Signifies that no patterns have been found yet in Builder
		Done, // Signifies that no successor is required after most recent Pattern

		// Keyword implementations

		Keyword_startprogram,

		Keyword_function,
		Keyword_structure,

		Keyword_wrest,
		Keyword_wprec,
		Keyword_precarrow,
		Keyword_wlabl,

		Keyword_import,
		Keyword_include,

		Keyword_for,
		Keyword_while,

		Keyword_if,
		Keyword_else,
		Keyword_else_comma,
		Keyword_else_colon,

		Keyword_return,
		Keyword_returns,

		Keyword_continue,
		Keyword_break,

		Keyword_let,

		// Literals

		Literal,

		// FreePattern implementations

		Argument,
		Filler,
		Declaration_base,
		Label,

		// Header implementations

		Header_Function,
		Header_Structure,

		// Body implementations

		Body_Function,
		Body_Structure,
		Body_Restrictions,
		Body_Precedence,

		// Expressions

		Expression,
		
		// Miscellaneous

		Declaration,
		Assignment,
		ForLoopHeader,
		NestedCode,

		_size

	};

	const char* getPatternIDString(PatternID ID);

}

#endif