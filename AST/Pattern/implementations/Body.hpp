#ifndef BODY_HPP
#define BODY_HPP

#include "./../../Builder/implementations/Builder_Body.hpp"
#include "./Wrapper.hpp"
#include "./../../Symbol/SymbolTableLinker.hpp"

// Bodies for functions and structures

namespace AST {

	using Body_Function = Wrapper<PatternID::Body_Function>;
	using Scanner_Body_Function = Scanner_Wrapper<Body_Function, Builder_Body_Function, SymbolTableLinker>;

	using ptr_Body_Function = ptr<Body_Function>;

	using Body_Structure = Wrapper<PatternID::Body_Structure>;
	using Scanner_Body_Structure = Scanner_Wrapper<Body_Structure, Builder_Body_Structure, SymbolTableLinker>;

	using ptr_Body_Structure = ptr<Body_Structure>;

	using Body_Restrictions = Wrapper<PatternID::Body_Restrictions>;
	using Scanner_Body_Restrictions = Scanner_Wrapper<Body_Restrictions, Builder_Body_Restrictions, SymbolTableLinker>;

	using ptr_Body_Restrictions = ptr<Body_Restrictions>;

	using Body_Precedence = Wrapper<PatternID::Body_Precedence>;
	using Scanner_Body_Precedence = Scanner_Wrapper<Body_Precedence, Builder_Body_Precedence, SymbolTableLinker>;

	using ptr_Body_Precedence = ptr<Body_Precedence>;

}

#endif