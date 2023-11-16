#ifndef AST_UTILITY_FUNCTIONS
#define AST_UTILITY_FUNCTIONS

#include "./../util/string.hpp"
#include "./../Code/Loader.hpp"
#include "./Pattern/PatternID.hpp"

#define AST_PAD_LENGTH 18
#define AST_POSITION_PAD_LENGTH 21
#define AST_SYMBOL_PAD_LENGTH 12

#define AST_VA_ARGS(...) __VA_ARGS__ // Useful macro

#define variable_alphabet "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_'"

namespace AST {

	string getPositionString(Code::Position start, Code::Position end, bool parenthesize = true);

	string getPositionString(Code::Loader& loader, int start, int end, bool parenthesize = true);

	bool isWhitespace(char);

	bool isNewline(char);

	string removeNewlines(const char*);

	string removeNewlines(string);

	string removePadding(const char*);

	string removePadding(string);

	string pad(const char*, int = AST_PAD_LENGTH);

	string pad(string, int = AST_PAD_LENGTH);

	bool isRangeEmpty(Code::Loader& loader, int start, int end); // Checks whether [start, end) contains nothing but whitespace

	int matchString(Code::Loader& loader, const char* str, int start);

	int matchString(Code::Loader& loader, managed_string& str, int start);

	int matchString_(Code::Loader& loader, const char* str, int& start); 

	int matchString_(Code::Loader& loader, managed_string& str, int& start);

	// Returns the smallest index following the match, or -1 if no match

	inline int matchString(Code::Loader& loader, managed_string& str, int start) { return matchString_(loader, str, start); }

	inline int matchString(Code::Loader& loader, const char* str, int start) { return matchString_(loader, str, start); }

	// Sets the start variable to the beginning index of str

	inline int matchString_(Code::Loader& loader, managed_string& str, int& start) { return matchString_(loader, str.c_str(), start); }

	inline int matchString_(Code::Loader& loader, const char* str, int& start) {

		int end = start;
		int str_idx = 0;

		while (str[str_idx]) {
			
			while (loader(end).is_whitespace) { if (str_idx == 0) start++; end++; if (end == loader.size()) return 0; }

			if (loader[end] == str[str_idx]) { end++; str_idx++; }
			else return -1;

		}

		return end;

	}

}

#endif