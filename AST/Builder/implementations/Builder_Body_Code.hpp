#ifndef BUILDER_BODY_CODE_HPP
#define BUILDER_BODY_CODE_HPP

#include "./../../../util/vstack.hpp"
#include "./../Builder.hpp"
#include "./../../Symbol/SymbolTableLinker.hpp"
#include "./../../CodePiece/CodePieceBuilder.hpp"

// Base class for Builder_Body_Function and Builder_Body_Structure

namespace AST {

	struct Builder_Body_Code : public Builder {

		Builder_Body_Code(BuilderID, Code::Loader& loader, int start, int end, int scope, SymbolTableLinker symbols);

		CodePieceBuilder code_pieces; // See "AST/Code/CodePiece.hpp"

		static bool isElseKW(PatternID);

		string printChildren(int);

	protected:

		static bool is_loop_body; // Used to signify whether the current Builder_Body_Code being evaluated is inside a loop body

		bool is_within_loop_body = false;
		bool next_is_loop_body = false;
		bool new_piece = true;
		bool require_assignment = false;
		bool allow_expression = true;
		bool allow_code_body = false;      
		bool is_control_flow = false;
		int elses_allowed = 0;
		int else_dist = 0;

		SymbolTableLinker vst_initial_table;
		SymbolTableLinker current_for_table;
		vstack<SymbolTableLinker> last_if_for_table;

		bool processPattern(ptr_Pattern);

		void processCodePieces();

		SymbolTableLinker getSymbols();

		bool verifyAssignment(int exp_end);

		void incrementElseDist();

	};

	using ptr_Builder_Body_Code = ptr<Builder_Body_Code>;

}

#endif