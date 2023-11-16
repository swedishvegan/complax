#ifndef FORLOOPHEADER_HPP
#define FORLOOPHEADER_HPP

#include "./Expression/Expression.hpp"

// Finds the FIRST for loop header that conforms to the following pattern: { varname } in { iterable expression }

namespace AST {

	struct ForLoopHeader : public Pattern {

		ptr_SymbolTable table;
		ptr_Expression iterator;

		ForLoopHeader();

		string getInfo(int);

	};

	using ptr_ForLoopHeader = ptr<ForLoopHeader>;

	struct Scanner_ForLoopHeader : public Scanner {

		Scanner_ForLoopHeader(Code::Loader& loader, int start, int end, int cur_best_start, SymbolTableLinker symbols);

	protected:

		Code::Loader& loader;
		int start, end;
		SymbolTableLinker symbols;

		ptr_Expression findIterator(int scan_start);

		ptr_ForLoopHeader getHeader(int, int, ptr_Expression);

	};

}

#endif