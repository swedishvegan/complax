#include "./Loader.hpp"

string Code::Position::toString() const { 

	auto l = line == 0 ? 1 : line;
	auto o = line == 0 ? 1 : offset;

	return std::to_string(l) + "." + std::to_string(o); 

}

bool Code::operator < (Position p, Position q) { return p.line < q.line || (p.line == q.line && p.offset < q.offset); }
bool Code::operator <= (Position p, Position q) { return p.line < q.line || (p.line == q.line && p.offset <= q.offset); }
bool Code::operator > (Position p, Position q) { return q < p; }
bool Code::operator >= (Position p, Position q) { return q <= p; }
bool Code::operator == (Position p, Position q) { return p.line == q.line && p.offset == q.offset; }
bool Code::operator != (Position p, Position q) { return !(p == q); }

Code::Loader::Loader(managed_string file_path) {

	loadCode(file_path);
	if (error.size() > 0) return;

	processCode();
	parseBrackets();

}

string Code::Loader::getErrorMessage() const { return error; }

int Code::Loader::size() const { return (int)code.size(); }

char Code::Loader::operator [] (int idx) const { if (idx >= code.size()) idx = (int)code.size() - 1; return code[idx]; }

Code::Position Code::Loader::operator () (int idx) const { return codep[idx % codep.size()]; }

int Code::Loader::getNextScopeStart(int idx, int scope) {

	for (auto bracket : brackets) if (bracket.idx >= idx && bracket.scope == scope) return bracket.idx == size() - 1 ? bracket.idx : bracket.idx + 1;
	return -1;

}

void Code::Loader::loadCode(managed_string file_path) {

	std::ifstream file(file_path.c_str(), std::ios::binary | std::ios::in);
	if (!file.is_open()) {

		error = "Failed to open file '" + file_path + "'.";
		return;

	}

	file.seekg(0, std::ios::beg);
	std::streampos fsize = file.tellg();
	file.seekg(0, std::ios::end);
	fsize = file.tellg() - fsize;

	char* buf = new char[(int)fsize + 1];
	file.seekg(0, std::ios::beg);
	file.read(buf, fsize);
	buf[fsize] = 0;

	code = buf;
	code = "{\n" + code + "\n}";

	delete[] buf;

}

void Code::Loader::processCode() {

	codep.resize((int)code.size());

	int line = 0;
	int offset = 1;
	int scope = 0;
	bool single_line_comment = false;
	bool multi_line_comment = false;
	char c_last = code[0];

	codep[0] = Position{ 0, line, offset, scope };

	for (int i = 1; i < code.size(); i++) {

		if (code[i - 1] == '\n') { line++; offset = 1; }
		else offset++;

		char c = code[i];
		bool is_comment = single_line_comment || multi_line_comment;
		bool multi_line_comment_ended = false;

		if (c == '\n') single_line_comment = false;
		if (c == '/' && c_last == '/' && !is_comment) { single_line_comment = true; codep[i - 1].is_whitespace = true; codep[i - 1].is_comment = true; }
		if (c == '/' && c_last == '*') { multi_line_comment = false; multi_line_comment_ended = true; }
		if (c == '*' && c_last == '/' && !is_comment) { multi_line_comment = true; codep[i - 1].is_whitespace = true; codep[i - 1].is_comment = true; }

		is_comment = single_line_comment || multi_line_comment || multi_line_comment_ended;
		bool is_whitespace = is_comment || c == ' ' || c == '\t' || c == '\r' || c == '\n';

		if (!is_whitespace) {

			if (c == '{') scope++;
			if (c == '}') scope--;

		}

		codep[i] = Position{ i, line, offset, scope, is_whitespace, is_comment };
		c_last = c;

		if (scope < 0 && i < code.size() - 1) return;

	}

	codep[size() - 1].is_comment = false;
	codep[size() - 1].is_whitespace = false;

}

void Code::Loader::parseBrackets() {

	int scope = -1;

	for (int i = 0; i < code.size(); i++) {

		if (codep[i].is_whitespace) continue;

		if (code[i] == '{') {

			scope++;
			brackets.push_back(Bracket{ i, scope, true });

		}

		if (code[i] == '}') {

			scope--;

			if (scope < 0 && i < code.size() - 1) {

				error = "Unexpected closing brace at " + codep[i].toString() + ".";
				return;

			}

			brackets.push_back(Bracket{ i, scope, false });

		}

	}

	if (scope > -1) for (int j = brackets.size() - 2; j >= 0; j--) if (brackets[j].scope == scope + 1 && brackets[j].is_left) {

		error = "Opening brace at " + codep[brackets[j].idx].toString() + " is never closed.";
		return;

	}

}