#ifndef BUILTINSYMBOLS_HPP
#define BUILTINSYMBOLS_HPP

#include "./SymbolTable.hpp"

// Utilities to define symbols that are built into the language's syntax by default (for example, operators and comparators)
// See BuiltInSymbols.hpp for all symbol definitions

#define OP_PREC 1.0    // p0 value for all operators
#define ADD_PREC 0.0   // p1 value for + operator
#define SUB_PREC 0.0   // p1 value for - operator
#define MUL_PREC 1.0   // p1 value for * operator
#define DIV_PREC 1.0   // p1 value for / operator
#define POW_PREC 2.0   // p1 value for ^ operator
#define MOD_PREC 3.0   // p1 value for % operator
#define BAND_PREC 4.0  // p1 value for & operator
#define BOR_PREC 4.0   // p1 value for | operator
#define BNOT_PREC 5.0  // p1 value for ! operator

#define COMP_PREC -1.0 // p0 value for all comparators == != >= <= > <

#define TLH_PREC -2.0  // LH p0 value for all type-related evaluators (is, islike, as, like, typeof)
#define TRH_PREC 2.0   // RH p0 value for all type-related evaluators

#define BOOL_PREC -3.0 // p0 value for all boolean operators
#define AND_PREC 0.0   // p1 value for 'and' operator
#define OR_PREC 0.0    // p1 value for 'or' operator
#define NOT_PREC 1.0   // p1 value for 'not' operator

#define SYS_PREC -4.0  // p0 value for sys functions

namespace AST {

	ptr_SymbolTable genDefaultSymbolTable(); // Generates all built-in symbols to be recognized by the compiler

	//ptr_SymbolTable genPrecedenceSymbolTable(); // Generates built-in constants that can be used within precedence bodies

	ptr_Symbol definePattern (

		const char* header,       // Example: "do stuff with {x} and {y}"
		const char* restrictions, // Example: "i,db" --> x must be integer, y could be decimal or bool (i = integer, d = decimal, b = bool, s = string, n = nothing)
		const char* description,
		SymbolID pattern_type, 
		Precedence p_lh = Precedence(), 
		Precedence p_rh = Precedence()

	);

}

#endif