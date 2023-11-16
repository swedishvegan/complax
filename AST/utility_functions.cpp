#include "./utility_functions.hpp"

string AST::getPositionString(Code::Position start, Code::Position end, bool parenthesize) { auto s = start.toString() + "-" + end.toString(); return parenthesize ? "(" + s + ")" : s; }

string AST::getPositionString(Code::Loader& loader, int start, int end, bool parenthesize) { return getPositionString(loader(start), loader(end)); }

bool AST::isWhitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

bool AST::isNewline(char c) { return c == '\r' || c == '\n'; }

string AST::removeNewlines(const char* str) {

	int i = 0;
	string s;

	while (str[i]) { if (!isNewline(str[i])) s += str[i]; i++; }

	return s;

}

string AST::removeNewlines(string str) { return removeNewlines(str.c_str()); }

string AST::removePadding(const char* str) {

	int i = 0;
	string s;

	int first_char = -1, last_char = -1;

	while (str[i]) {

		if (!isWhitespace(str[i])) { last_char = i; if (first_char < 0) first_char = i; }
		i++;

	}

	if (first_char < 0) return "";

	for (i = first_char; i <= last_char; i++) s += str[i];

	return s;

}

string AST::removePadding(string str) { return removePadding(str.c_str()); }

string AST::pad(const char* str, int pad_num) { return pad(string(str), pad_num); }

string AST::pad(string str, int pad_num) {

	int req_space = pad_num - (int)str.size();
	if (req_space < 0) return str.substr(0, pad_num - 3) + "...";

	for (int i = 0; i < req_space; i++) str += " ";
	return str;

}

bool AST::isRangeEmpty(Code::Loader& loader, int start, int end) { for (int i = start; i < end; i++) if (!loader(i).is_whitespace) return false; return true; }