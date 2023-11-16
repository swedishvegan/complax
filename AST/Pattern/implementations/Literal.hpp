#ifndef LITERAL_HPP
#define LITERAL_HPP

#include "./../../../util/string.hpp"
#include "./../Pattern.hpp"
#include "./../../Type/Type.hpp"

// Detects literals, such as numbers, true/false, strings, etc.

namespace AST {

	struct Literal : public Pattern {

		managed_string info;
		TypeID type = Type::Unknown;

		Literal();

		string getInfo(int);

	};

	using ptr_Literal = ptr<Literal>;

	struct Scanner_Literal : public Scanner {

		Scanner_Literal(Code::Loader& loader, int start, int end, int = 0); // Fourth variable is a dummy to make Scanner_Literal work nicely with macros

	};

}

#endif