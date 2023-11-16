#include "./Builder_Body_Function.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"

AST::Builder_Body_Function::Builder_Body_Function(Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols)
	: Builder_Body_Code(BuilderID::Builder_Body_Function, loader, start, end, scope, symbols) {

	is_within_loop_body = is_loop_body;
	
	build();
	if (!error.error) processCodePieces();

	is_loop_body = is_within_loop_body;

	Scanner_Expression::cleanup();
	code_pieces.finish();

}

AST_DECL_SUCCESSORS(Builder_Body_Function) {
	
	if (require_assignment) { AST_SUCC_ALLOW(Assignment); require_assignment = false; return; }

	if (new_piece) {

		if (allow_expression) AST_SUCC_ALLOW(Expression);

		AST_SUCC_ALLOW(Declaration);

		//AST_SUCC_ALLOW(Keyword_for);
		AST_SUCC_ALLOW(Keyword_while);

		AST_SUCC_ALLOW(Keyword_if);
		if (elses_allowed > 0 && else_dist == 1) { AST_SUCC_ALLOW(Keyword_else); AST_SUCC_ALLOW(Keyword_else_comma); AST_SUCC_ALLOW(Keyword_else_colon); }

		AST_SUCC_ALLOW(Keyword_return);
		
		if (allow_code_body) AST_SUCC_ALLOW(Body_Function);

		if (!is_control_flow) AST_SUCC_ALLOW(Done);

		AST_SUCC_ALLOW(Keyword_continue); 
		AST_SUCC_ALLOW(Keyword_break);

		return;

	}

	if (AST_SUCC_ISTYPE(Declaration) || AST_SUCC_ISTYPE(Assignment)) AST_SUCC_ALLOW(Expression);

	if (AST_SUCC_ISTYPE(Keyword_while) || AST_SUCC_ISTYPE(Keyword_if) || AST_SUCC_ISTYPE(Keyword_return)) AST_SUCC_ALLOW(Expression);

	AST_SUCC_CASE(Keyword_for) AST_SUCC_ALLOW(ForLoopHeader);
	
}