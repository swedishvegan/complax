#ifndef PATTERN_HPP
#define PATTERN_HPP

#include "./../../util/ptr.hpp"
#include "./../../Code/Loader.hpp"
#include "./PatternID.hpp"
#include "./../../util/CompileError.hpp"
#include "./../../util/Printable.hpp"
#include "./../utility_functions.hpp"

// Abstract base classes that are used in Builder implementations and derived from elsewhere in the codebase

namespace AST {
	
	// A Pattern is the result of a Scanner's scanning

	struct Pattern : public Printable {

		PatternID ID;
		Code::Position start, end;
		CompileError error;

		int precedence = 0; // Used in special instances to force certain Patterns to take precedence in others during Building

		Pattern(PatternID ID);

		virtual ~Pattern();

		string toString(int);

		virtual string getInfo(int); // Additional info for printing Pattern, called in toString() function

	};

	using ptr_Pattern = ptr<Pattern>;

	// A Scanner scans for a Pattern

	struct Scanner {

		ptr_Pattern result = nullptr; // Scanner returns result by storing it into this variable in the constructor, or nullptr if scanning fails

		Scanner(Code::Loader& loader, int start, int end, int cur_best_start); // Scanner scans from range [start, end), should return failure if the pattern goes outside of this range or starts after cur_best_start

		virtual ~Scanner();
		
	};

}

#endif