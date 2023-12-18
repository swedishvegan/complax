#include "./PatternID.hpp"

const char* _PatternID_strings[] = {

	"None",
	"Done",

	// Keyword implementations

	"Keyword_startprogram",

	"Keyword_function",
	"Keyword_structure",
	
	"Keyword_wrest",
	"Keyword_wprec",
	"Keyword_precarrow",
	"Keyword_wlabl",

	"Keyword_import",
	"Keyword_include",

	"Keyword_for",
	"Keyword_while",

	"Keyword_if",
	"Keyword_else",
	"Keyword_else_comma",
	"Keyword_else_colon",

	"Keyword_return",
	"Keyword_returns",

	"Keyword_continue",
	"Keyword_break",

	"Keyword_let",

	// Literals

	"Literal",

	// FreePattern implementations

	"Argument",
	"Filler",
	"Declaration_base",
	"Label",

	// Header implementations

	"Header_Function",
	"Header_Structure",

	// Body implementations

	"Body_Function",
	"Body_Structure",
	"Body_Restrictions",
	"Body_Precedence",

	// Expressions

	"Expression",

	// Miscellaneous

	"Declaration",
	"Assignment",
	"ForLoopHeader",
	"NestedCode",

};

const char* AST::getPatternIDString(PatternID ID) { return _PatternID_strings[(int)ID]; }