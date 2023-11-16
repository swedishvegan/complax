#ifndef CODEPIECE_HPP
#define CODEPIECE_HPP

#include "./../../util/string.hpp"
#include "./../../util/ptr.hpp"
#include "./../../util/Printable.hpp"

/*

CodePieces are used to compartmentalize code inside of structure/ function bodies into logical, 
hierarchical "pieces" that make it much easier later on to perform instantiation and type checking.

A CodePiece is simply a single building block of code, i.e. "[variable] = [expression]" or
"[function call]" or "for [var] in [iterator]". The Builder implementations don't naturally see the
code in terms of these blocks; they just see the individual pieces, one Pattern at a time. Thus, some
extra processing is needed to represent the code in this type of structure.

A CodePieceBuilder can be thought of as an extra layer of abstraction on top of a Builder. A Builder
reads in raw characters from a text file and builds a hierarchy of Patterns. Then, a CodePieceBuilder
reads in Patterns found by the Builder and builds a hierarchy of CodePieces.

*/

namespace AST {

	enum class CodePieceID { PatternMatch, Declaration, Assignment, Body, NestedCode, For, While, If, Else, Return, LoopLogic };

	struct CodePiece;
	using ptr_CodePiece = ptr<CodePiece>;

	struct CodePiece : public Printable {

		CodePieceID ID;
		ptr_CodePiece owner;

		CodePiece(CodePieceID);

		void removeCircularDependencies(); // Called in CodePieceBuilder during cleanup in order to avoid memory leaks (two smart pointers referencing each other)

		string toString(int);

	};

}

#endif