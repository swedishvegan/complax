#include "./../../Symbol/Symbol.hpp"
#include "./Literal.hpp"

AST::Literal::Literal() : Pattern(PatternID::Literal) { }

string AST::Literal::getInfo(int alignment) { return removePadding(removeNewlines(info.c_str())) + "\n" + Type(type).print(alignment + 1, false); }

AST::Scanner_Literal::Scanner_Literal(Code::Loader& loader, int start, int end, int) : Scanner(loader, start, end, 0) {

	static const char* typename_kw[] = {

		"nothing",

		"integer",
		"decimal",
		"boolean", 
		"ascii",
		"string",

		"array",

	};

	static TypeID typename_kw_ID[] = {

		Type::Nothing,

		Type::Integer,
		Type::Decimal,
		Type::Bool,
		Type::Ascii,
		Type::String,

		Type::Array,

	};

	static const int num_kw = sizeof(typename_kw) / sizeof(char*);

	for (int i = 0; i < num_kw; i++) {

		int typename_kw_match = matchString(loader, typename_kw[i], start);

		if (typename_kw_match < 0) continue;

		ptr_Literal literal = new Literal();

		literal->start = loader(start);
		literal->end = loader(typename_kw_match);
		literal->info = typename_kw[i];
		literal->type = typename_kw_ID[i];
		literal->is_keyword = true;

		result = literal.cast<Pattern>();
		return;

	}

	static const char* true_kw = "true";
	static const char* false_kw = "false";

	int kw_true_match = matchString(loader, true_kw, start);
	int kw_false_match = (kw_true_match < 0) ? kw_false_match = matchString(loader, false_kw, start) : -1;

	if (kw_true_match >= 0 || kw_false_match >= 0) {

		ptr_Literal literal = new Literal();

		literal->start = loader(start);
		literal->end = loader(kw_true_match >= 0 ? kw_true_match : kw_false_match);
		literal->info = kw_true_match >= 0 ? true_kw : false_kw;
		literal->type = Type::Bool;
		literal->is_keyword = true;
		
		result = literal.cast<Pattern>();
		return;

	}

	int true_start = start;
	while (loader(true_start).is_whitespace) true_start++;

	char delimiter = loader[true_start];

	if (delimiter == '"' || delimiter == '\'') {

		true_start++;

		bool ignore_next = false;
		managed_string kw = "";

		while (true) {

			if (true_start == loader.size()) return;

			if (loader(true_start).is_comment) { true_start++; continue;}

			char c = loader[true_start];

			if (c == delimiter && !ignore_next) break;

			else if (c == '\\' && !ignore_next) ignore_next = true;

			else {

				if (c == 'n' && ignore_next) c = '\n';
				if (c == 't' && ignore_next) c = '\t';
				
				kw += c; 
				ignore_next = false;
				
			}

			true_start++;

		}
		
		ptr_Literal literal = new Literal();

		literal->start = loader(start);
		literal->end = loader(true_start + 1);
		literal->info = kw;
		literal->type = (delimiter == '"') ? Type::String : Type::Ascii;

		if (literal->type == Type::Ascii && kw.size() != 1) {

			literal->error.error = true;
			literal->error.info = "Ascii literal at " + loader(start).toString() + " contains " + string(
				(kw.size() == 0)
					? "no characters."
					: "too many characters."
			);

		}

		result = literal.cast<Pattern>();
		return;

	}

	bool is_fp = false;
	bool has_exp = false;

	bool minus_allowed = true;
	bool dot_allowed = true;
	bool exp_allowed = false;
	bool is_valid = false;

	char c_last = ' ';

	int idx = true_start;
	int last_valid_idx = true_start - 1;
	
	while (true) {

		char c = loader[idx];

		if (c == '-') {

			if (minus_allowed) { minus_allowed = false; is_valid = false; }
			else break;

		}
		else if (c == '.') {

			if (dot_allowed) { is_fp = true; dot_allowed = false; }
			else break;

		}
		else if (c == 'e' || c == 'E') {

			if (exp_allowed) { is_fp = true; has_exp = true; minus_allowed = true; dot_allowed = false; exp_allowed = false; is_valid = false; }
			else break;

		}
		else if ((unsigned int)c >= (unsigned int)'0' && (unsigned int)c <= (unsigned int)'9') {

			minus_allowed = false;
			if (!has_exp) exp_allowed = true;
			is_valid = true;

		}
		else break;

		c_last = c;

		if (is_valid) last_valid_idx = idx;
		idx++;

	}

	if (last_valid_idx < true_start || last_valid_idx + 1 > end) return;
	
	managed_string kw = "";
	for (int i = true_start; i <= last_valid_idx; i++) kw += loader[i];

	ptr_Literal literal = new Literal();
	
	literal->start = loader(start);
	literal->end = loader(last_valid_idx + 1);
	literal->info = kw;
	literal->type = is_fp ? Type::Decimal : Type::Integer;
	
	result = literal.cast<Pattern>();
	return;

}