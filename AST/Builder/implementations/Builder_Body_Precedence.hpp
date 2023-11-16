#ifndef BUILDER_BODY_PRECEDENCE_HPP
#define BUILDER_BODY_PRECEDENCE_HPP

#include "./../Builder.hpp"
#include "./../../Symbol/SymbolTableLinker.hpp"

namespace AST {

	struct Builder_Body_Precedence : public Builder {

		Builder_Body_Precedence(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols);

		string printChildren(int alignment);

		Precedence sym_precedence;

	protected:

		int exp_count = 0;

		void generateSuccessors(PatternID cur_ID);

		bool processPattern(ptr_Pattern);

		SymbolTableLinker getSymbols();

	};

	using ptr_Builder_Body_Precedence = ptr<Builder_Body_Precedence>;

}

#endif