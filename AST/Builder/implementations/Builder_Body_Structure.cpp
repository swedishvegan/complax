#include "./Builder_Body_Structure.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"

AST::Builder_Body_Structure::Builder_Body_Structure(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols)
	: Builder_Body_Code(BuilderID::Builder_Body_Structure, loader, start, end, scope, symbols) {

	build();
	if (!error.error) processCodePieces();

	Scanner_Expression::cleanup();
	code_pieces.finish();

}

AST_DECL_SUCCESSORS(Builder_Body_Structure) {
	
	if (AST_SUCC_ISTYPE(Expression) || AST_SUCC_ISTYPE(None)) {

		AST_SUCC_ALLOW(Declaration);
		AST_SUCC_ALLOW(Done);

	}

	AST_SUCC_CASE(Declaration) AST_SUCC_ALLOW(Expression);

}