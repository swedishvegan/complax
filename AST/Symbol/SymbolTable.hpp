#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include "./Symbol.hpp"
#include "./../../util/vec.hpp"
#include "./../../util/Printable.hpp"

namespace AST {

	struct SymbolTable;
	struct SymbolTableLinker;

	using SymbolList = managed_vec<ptr_Symbol>;
	using ptr_SymbolTable = ptr<SymbolTable>;

	struct SymbolTable : public Printable {

		SymbolList functions;
		SymbolList structures;
		SymbolList variables;

		managed_vec<ptr_SymbolTable> includes;

		bool is_header = false; // Set to true if the table is created by a Builder_Header

		SymbolTable(int scope_size); // The argument is used as an upper bound for how many symbols could possibly be created in this table

		SymbolList& getSymbolList(SymbolID);

		bool conflictsWith(Symbol&);

		void add(ptr_Symbol); // This function does not check for conflicts before adding the Symbol, so this must be done manually with the conflictsWith() function

		void include(ptr_SymbolTable); // Checks for table conflicts but not for symbol conflicts, but if there is a conflict, it will likely cause an error later on during expression parsing

		void include(SymbolTableLinker);

		string toString(int);

	protected:

		size_t num_buckets;

	};

}

#endif