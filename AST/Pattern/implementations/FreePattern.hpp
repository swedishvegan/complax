#ifndef FREEPATTERN_HPP
#define FREEPATTERN_HPP

#include "./../../../util/string.hpp"
#include "./../Pattern.hpp"

// User defined names, like variables, arguments, labels, etc.
//     See line 105 for FreePattern implementations
//     See line 41 for explanation of how Scanner_FreePattern objects work

namespace AST {

	struct FreePattern : public Pattern {

		managed_string name;

		FreePattern(PatternID ID);

		string getInfo(int);

	};

	using ptr_FreePattern = ptr<FreePattern>;

	struct Alphabet {

		managed_string chars;
		bool inclusive = false; // If true, the chars represent the list of allowed characters; if false, they represent the list of banned characters

	};

	template <typename FreePatternType>
	struct Scanner_FreePattern : public Scanner {

		Scanner_FreePattern(

			Code::Loader& loader,
			int start,
			int end,
			int cur_best_start,
			char primer,              // Character where Scanner should start recording the user-defined name
			const char* terminator,   // List of characters where Scanner should stop recording the user-defined name
			Alphabet alphabet,        // An alphabet of either allowed characters or banned characters, depending on whether alphabet is flagged as inclusive
			bool ignore_whitespace,   // Whether of not whitespace is saved into the name string
			bool include_terminator   // Whether or not the terminator is counted as part of the Pattern, as reflected in the (start, end) range (normally should be set to true)

		) : Scanner(loader, start, end, cur_best_start) {
			
			while (loader(start).is_whitespace) start++;

			if (start >= end || start > cur_best_start) return;
			if (primer && loader[start] != primer) return;

			int i = start + (int)(bool)primer; if (i >= end) return;

			managed_string name;

			while (i < end) {

				if (loader(i).is_comment) { i++; continue; }
				if (stringContainsChar(terminator, loader[i])) break;

				bool in_alphabet = stringContainsChar(alphabet.chars.c_str(), loader[i]);

				if (!ignore_whitespace || !loader(i).is_whitespace) {

					if (in_alphabet && !alphabet.inclusive) return;
					if (!in_alphabet && alphabet.inclusive) return;

					name += loader[i];

				}

				i++;

			}

			if (name.size() == 0) return;
			if (i + (int)include_terminator > end) return;
			if (!stringContainsChar(terminator, loader[i])) return;

			if (include_terminator) i++;

			ptr<FreePatternType> free_pattern = new FreePatternType();

			free_pattern->start = loader(start);
			free_pattern->end = loader(i);
			free_pattern->name = name;

			result = free_pattern.template cast<Pattern>();

		}

	protected:

		bool stringContainsChar(const char* str, char c) {

			int i = 0;

			while (str[i]) { if (str[i] == c) return true; i++; }
			return false;

		}

	};

#define AST_DECL_FREEPATTERN(classname) \
\
struct classname : public FreePattern { classname(); }; \
struct Scanner_##classname : public Scanner_FreePattern<classname> { Scanner_##classname(Code::Loader& loader, int start, int end, int cur_best_start); }

	// FreePattern declarations begin here, see line 14 of FreePattern.cpp for constructor definitions

	AST_DECL_FREEPATTERN(Argument);
	AST_DECL_FREEPATTERN(Filler);
	AST_DECL_FREEPATTERN(Declaration_base);
	AST_DECL_FREEPATTERN(Label);

}

#endif