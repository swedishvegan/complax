#ifndef SYMBOLTABLELINKER_HPP
#define SYMBOLTABLELINKER_HPP

#include <initializer_list>
#include "./SymbolTable.hpp"
#include "./SymbolSearchTreeBase.hpp"
#include "./../../util/Printable.hpp"
#include "./../../util/vec.hpp"

// Links together SymbolTables for the purpose of Expression parsing and Symbol matching

namespace AST {

	struct Builder;
	struct SymbolTableLinker;
	
	using ptr_SymbolTableLinker = ptr<SymbolTableLinker>;

	struct SymbolTableLinker : public Printable {
		
		struct Iterator;

		ptr_SymbolTable table = nullptr; 
		ptr_SymbolTableLinker next = nullptr;

		ptr_SymbolSearchTreeBase pattern_match_search_tree; // Expression_Scanner iterates over search trees, instead of iterating over the symbol tables directly
		ptr_SymbolSearchTreeBase variable_search_tree;

		SymbolTableLinker();
		SymbolTableLinker(std::initializer_list<ptr_SymbolTable>);
		SymbolTableLinker(managed_vec<ptr_SymbolTable>&);

		void clear();

		SymbolTableLinker deepCopy();

		SymbolTableLinker linkWith(ptr_SymbolTable table);
		SymbolTableLinker linkWith(SymbolTableLinker linker);
		
		SymbolTableLinker attachTree(ptr_SymbolSearchTreeBase search_tree);

		Iterator iterate(SymbolID); // Used to conveniently iterate over every Symbol of a particular type within the SymbolTableLinker

		string toString(int);

		struct Iterator { // Example usage: auto it = linker.iterate(SymbolID::Variable); while (!it.finished()) { ... it = it.next(); }

			ptr_Symbol operator () (); // Returns containing symbol

			Iterator next(); // Next symbol

			bool finished(); // Whether iterating is finished
			
		protected:

			ptr_Symbol sym = nullptr;
			SymbolTableLinker* owner = nullptr; // Iterators are intended to be temporary objects and the lifetime of an Iterator should not exceed lifetime of its owner
			int idx = 0;

			friend struct SymbolTableLinker;

		};

	protected:

		void link(managed_vec<ptr_SymbolTable>&);

		static void linkWith_(SymbolTableLinker&, ptr_SymbolTable);

		friend struct Builder; // Allows Builder to call empty SymbolTableLinker() constructor
	
	};

}

#endif