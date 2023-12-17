#ifndef CODEPIECE_IMPLEMENTATIONS_HPP
#define CODEPIECE_IMPLEMENTATIONS_HPP

#include "./CodePiece.hpp"
#include "./../Pattern/implementations/ForLoopHeader.hpp"

// Note: CodePiece implementations store raw pointers to Patterns, Nodes, and Symbols in order to avoid circular dependencies
// They do not own this memory and thus it is not necessary to deallocate it within the implementation destructors

namespace AST {

	struct CodePiece_PatternMatch : public CodePiece { Expression* pattern_match = nullptr; CodePiece_PatternMatch(); };
	using ptr_CodePiece_PatternMatch = ptr<CodePiece_PatternMatch>;

	struct CodePiece_Assignment;
	using ptr_CodePiece_Assignment = ptr<CodePiece_Assignment>;

	struct CodePiece_Declaration : public CodePiece { Symbol* new_var = nullptr; ptr_CodePiece_Assignment assignment; CodePiece_Declaration(); };
	using ptr_CodePiece_Declaration = ptr<CodePiece_Declaration>;

	struct CodePiece_Assignment : public CodePiece {

		Expression* LH_assign = nullptr; // Either a VariableNode, StructureMemberNode, or PatternMatchNode
		Symbol* LH_declare = nullptr;    // LH symbol for when the assignment is part of a declaration
		
		Expression* RH = nullptr;

		CodePiece_Assignment();

	};

	struct CodePiece_Body : public CodePiece { 
		
		Pattern* body = nullptr; 
		
		CodePiece_Body(); 

		managed_vec<ptr_CodePiece>& pieces();
	
	};
	using ptr_CodePiece_Body = ptr<CodePiece_Body>;

	struct CodePiece_NestedCode : public CodePiece {

		ptr_CodePiece header;
		ptr_CodePiece body_pt1;
		ptr_CodePiece body_pt2; // Usually nullptr, with the one exception being if/ else statements

		CodePiece_NestedCode();

	protected:

		bool header_done = false;
		bool body_pt1_done = false;
		bool body_pt2_done = false;

		friend struct CodePieceBuilder;

	};
	using ptr_CodePiece_NestedCode = ptr<CodePiece_NestedCode>;

	struct CodePiece_For : public CodePiece { ForLoopHeader* header = nullptr; CodePiece_For(); };
	using ptr_CodePiece_For = ptr<CodePiece_For>;

	struct CodePiece_While : public CodePiece { Expression* condition = nullptr; CodePiece_While(); };
	using ptr_CodePiece_While = ptr<CodePiece_While>;

	struct CodePiece_If : public CodePiece { Expression* condition = nullptr; CodePiece_If(); };
	using ptr_CodePiece_If = ptr<CodePiece_If>;

	struct CodePiece_Else : public CodePiece { CodePiece_Else(); };
	using ptr_CodePiece_Else = ptr<CodePiece_Else>;

	struct CodePiece_Return : public CodePiece { Expression* return_expression = nullptr; CodePiece_Return(); };
	using ptr_CodePiece_Return = ptr<CodePiece_Return>;

	struct CodePiece_LoopLogic : public CodePiece { bool is_continue; CodePiece_LoopLogic(); }; // Either a "continue" or a "break"
	using ptr_CodePiece_LoopLogic = ptr<CodePiece_LoopLogic>;

}

#endif