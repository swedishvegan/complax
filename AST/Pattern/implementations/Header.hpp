#ifndef HEADER_HPP
#define HEADER_HPP

#include "./../../Builder/implementations/Builder_Header.hpp"
#include "./Wrapper.hpp"

// Headers for functions and structures

namespace AST { 
	
	using Header_Function = Wrapper<PatternID::Header_Function>; 
	using Scanner_Header_Function = Scanner_Wrapper<Header_Function, Builder_Header_Function>;

	using ptr_Header_Function = ptr<Header_Function>;

	using Header_Structure = Wrapper<PatternID::Header_Structure>;
	using Scanner_Header_Structure = Scanner_Wrapper<Header_Structure, Builder_Header_Structure>;

	using ptr_Header_Structure = ptr<Header_Structure>;
	
}

#endif