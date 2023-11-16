#ifndef BUILDER_BODY_FUNCTION_HPP
#define BUILDER_BODY_FUNCTION_HPP

#include "./Builder_Body_Code.hpp"

namespace AST {

	struct Builder_Body_Function : public Builder_Body_Code {

		Builder_Body_Function(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols);

	protected:

		void generateSuccessors(PatternID cur_ID);

	};

	using ptr_Builder_Body_Function = ptr<Builder_Body_Function>;

}

#endif