#ifndef DECLARATION_HPP
#define DECLARATION_HPP

#include "./FreePattern.hpp"
#include "./../../Symbol/Symbol.hpp"

namespace AST {

	struct Declaration : public Pattern {

		ptr_Symbol sym;

		Declaration();

		string getInfo(int);

	};

	using ptr_Declaration = ptr<Declaration>;

	struct Scanner_Declaration : public Scanner {

		Scanner_Declaration(Code::Loader& loader, int start, int end, int cur_best_start);

	};

}

#endif