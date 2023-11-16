#ifndef CODEPIECEBUILDER_HPP
#define CODEPIECEBUILDER_HPP

#include "./CodePiece.hpp"
#include "./../Pattern/Pattern.hpp"
#include "./../../util/CompileError.hpp"

namespace AST {

	struct CodePieceBuilder : public Printable {

		CompileError error;
		managed_vec<ptr_CodePiece> pieces;

		CodePieceBuilder();

		void scanNextPattern(ptr_Pattern);

		void finish(); // Tidies everything up, including removing circular dependencies which would cause memory leaks

		string toString(int);

	protected:

		ptr_CodePiece cur;

		ptr_CodePiece getNewCodePiece(ptr_Pattern, ptr_CodePiece attach_to = nullptr);

		void pushCurrentCodePiece(bool allow_else = true);

	};

}

#endif