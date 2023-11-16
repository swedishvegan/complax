#ifndef BUILDER_BODY_RESTRICTONS_HPP
#define BUILDER_BODY_RESTRICTONS_HPP

#include "./../Builder.hpp"
#include "./../../Symbol/SymbolTableLinker.hpp"
#include "./../../Type/Restrictions.hpp"

namespace AST {

	struct Builder_Body_Restrictions : public Builder {

		managed_map<ptr_Symbol, vec<ptr_Expression>> restrictions;

		Builder_Body_Restrictions(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols);

		string printChildren(int alignment);

	protected:

		ptr_SymbolTable header_table;
		
		bool expecting_var = true;
		bool expecting_type = false;

		ptr_Symbol cur_var;

		void generateSuccessors(PatternID cur_ID);

		bool processPattern(ptr_Pattern);

		SymbolTableLinker getSymbols();

		bool findHeaderTable();

	};

	using ptr_Builder_Body_Restrictions = ptr<Builder_Body_Restrictions>;

}

#endif