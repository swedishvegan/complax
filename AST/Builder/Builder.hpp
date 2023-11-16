#ifndef BUILDER_HPP
#define BUILDER_HPP

#include "./../../util/string.hpp"
#include "./../../util/ptr.hpp"
#include "./../../util/vec.hpp"
#include "./../../util/CompileError.hpp"
#include "./../Pattern/Pattern.hpp"
#include "./../../Code/Loader.hpp"
#include "./BuilderID.hpp"
#include "./../Symbol/SymbolTableLinker.hpp"
#include "./../Pattern/implementations/Expression/Node.hpp"

// A Builder is a base class designed to build various types of ASTs

namespace AST {

	struct Builder : public Printable {

		BuilderID ID;       // Builder type, used for typecasting
		int main_scope = 0; // Main scope that Builder lives in
		int start, end;     // Builder exists in the range [start, end)
		CompileError error;

		managed_vec<ptr_Pattern> patterns;

		bool finished = false;

		Code::Loader& loader;

		ptr_SymbolTable table; // Every Builder creates a SymbolTable and fills it up as it scans for Patterns

		ptr_PatternMatchSearchTree pattern_match_search_tree; // See "AST/Symbol/SymbolSearchTree.hpp"
		ptr_VariableSearchTree variable_search_tree;
	
		Builder(BuilderID, Code::Loader&, int start, int end); // Builds the subset of the code in the range [start, end)

		void build(); // When Builder is implemented, this function should be called in the constructor

		virtual ~Builder();

		string toString(int);

		virtual string printChildren(int);

	protected:

#define AST_BUILDER_MAX_SUCCESSORS 16
		
		PatternID successor_IDs[AST_BUILDER_MAX_SUCCESSORS];
		
		int next_start = 0; // Index where next Pattern is expected to start
		PatternID cur_ID = PatternID::None; // Current Pattern being parsed in trySuccessor(), available for reference in child Builders

		SymbolTableLinker symbols; // Don't worry about this

		virtual void generateSuccessors(PatternID) = 0; // Informs the Builder which successors are legal given the current Pattern, and fills successor_IDs accordingly (Note: if no successor is required, successor_IDs[0] should be set to PatternID::Done)

		void checkForFinalErrors();

		void allowSuccessor(PatternID);

		void getNextPattern(PatternID); // Attempts to find a successor to the current Pattern

		int comparePatterns(ptr_Pattern, ptr_Pattern); // Returns -1 if LH takes precedence, 1 if RH takes precedence, 0 if both take equal precedence

		ptr_Pattern trySuccessor(PatternID, int); // Tries to match the pattern specified by the PatternID, and returns nullptr if attempt fails

		virtual bool processPattern(ptr_Pattern); // Implementation-specific post-processing after Pattern has been parsed, returning a bool signal indicating whether the current Pattern should be added to the Builder

		virtual SymbolTableLinker getSymbols(); // Implementation specific function for getting all current relevant symbols to pass down to child Builders

	};

	using ptr_Builder = ptr<Builder>;

}

#endif