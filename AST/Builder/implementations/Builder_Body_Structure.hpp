#ifndef BUILDER_BODY_STRUCTURE_HPP
#define BUILDER_BODY_STRUCTURE_HPP

#include "./Builder_Body_Code.hpp"

namespace AST {

	struct Builder_Body_Structure : public Builder_Body_Code {

		Builder_Body_Structure(Code::Loader& loader, int start, int end, int scope,SymbolTableLinker symbols);

	protected:

		void generateSuccessors(PatternID cur_ID);

	};

	using ptr_Builder_Body_Structure = ptr<Builder_Body_Structure>;

}

#endif