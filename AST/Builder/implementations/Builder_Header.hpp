#ifndef BUILDER_HEADER_HPP
#define BUILDER_HEADER_HPP

#include "./../Builder.hpp"
#include "./../../Pattern/PatternID.hpp"

// Builds headers for functions, structures, operators, etc.
//     Builder_Header implementations begin on line 26

namespace AST {

	struct Builder_Header : public Builder {

		ptr_HeaderSymbol sym; // You can tell what type of Header this Builder object is by checking sym.ID

		Builder_Header(Code::Loader& loader, int start, int end, int scope, SymbolID symbol_type);
		
	protected:

		void generateSuccessors(PatternID cur_ID);

		bool processPattern(ptr_Pattern);

		void noFillersError();

	};

	using ptr_Builder_Header = ptr<Builder_Header>;

	// Builder_Header implementations begin here, constructors are implemented starting on line 58 of Builder_Header.cpp

	struct Builder_Header_Function : public Builder_Header { Builder_Header_Function(Code::Loader& loader, int start, int end, int scope); };
	
	using ptr_Builder_Header_Function = ptr<Builder_Header_Function>;

	struct Builder_Header_Structure : public Builder_Header { Builder_Header_Structure(Code::Loader& loader, int start, int end, int scope); };

	using ptr_Builder_Header_Structure = ptr<Builder_Header_Structure>;

}

#endif