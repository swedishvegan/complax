#include "./../../util/vec.hpp"
#include "./../../AST/utility_functions.hpp"
#include "./BuiltInSymbols.hpp"

AST::ptr_SymbolTable AST::genDefaultSymbolTable() {

	ptr_SymbolTable def_table = new SymbolTable(100);

	const char* int_restrictions =     "iI,iI";
	const char* float_restrictions =   "fF,fF";
	const char* number_restrictions =  "iIfF,iIfF";
	const char* bool_restrictions =    "b,b";
	const char* literal_restrictions = "iIfFbs,iIfFbs";

	const char* op_desc =    "Operator";
	const char* comp_desc =  "Comparator";
	const char* teval_desc = "TypeEvaluator";
	const char* tcast_desc = "Typecast";
	const char* tcomp_desc = "TypeComparator";
	const char* range_desc = "Range";
	const char* sys_desc =   "Sys";
	const char* arr_desc =   "ArrayLogic";

	def_table->add(definePattern("{x} + {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} + {y}",              "s,s",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} + {y}",              "c,c",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} + {y}",              "c,is",                op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} + {y}",              "is,c",                op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} + {y}",              "a,a",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, ADD_PREC),   Precedence(OP_PREC, ADD_PREC)));
	def_table->add(definePattern("{x} - {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, SUB_PREC),   Precedence(OP_PREC, SUB_PREC)));
	def_table->add(definePattern("{x} - {y}",              "c,c",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, SUB_PREC),   Precedence(OP_PREC, SUB_PREC)));
	def_table->add(definePattern("{x} - {y}",              "c,i",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, SUB_PREC),   Precedence(OP_PREC, SUB_PREC)));
	def_table->add(definePattern("{x} - {y}",              "i,c",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, SUB_PREC),   Precedence(OP_PREC, SUB_PREC)));
	def_table->add(definePattern("{x} * {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, MUL_PREC),   Precedence(OP_PREC, MUL_PREC)));
	def_table->add(definePattern("{x} / {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, DIV_PREC),   Precedence(OP_PREC, DIV_PREC)));
	def_table->add(definePattern("{x} ^ {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, POW_PREC),   Precedence(OP_PREC, POW_PREC)));
	def_table->add(definePattern("{x} % {y}",              "id,id",               op_desc,    SymbolID::Function,   Precedence(OP_PREC, MOD_PREC),   Precedence(OP_PREC, MOD_PREC)));
	def_table->add(definePattern("{x} & {y}",              "i,i",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, BAND_PREC),  Precedence(OP_PREC, BAND_PREC)));
	def_table->add(definePattern("{x} | {y}",              "i,i",                 op_desc,    SymbolID::Function,   Precedence(OP_PREC, BOR_PREC),   Precedence(OP_PREC, BOR_PREC)));
	def_table->add(definePattern("!{x}",                   "i",                   op_desc,    SymbolID::Function,   Precedence(OP_PREC, BNOT_PREC),  Precedence(OP_PREC, BNOT_PREC)));

	def_table->add(definePattern("{x} == {y}",             "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} == {y}",             "b,b",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} == {y}",             "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} == {y}",             "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} == {y}",             "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} == {y}",             "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "b,b",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} != {y}",             "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));	
	def_table->add(definePattern("{x} > {y}",              "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} > {y}",              "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} > {y}",              "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} > {y}",              "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} > {y}",              "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));	
	def_table->add(definePattern("{x} < {y}",              "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} < {y}",              "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} < {y}",              "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} < {y}",              "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} < {y}",              "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));	
	def_table->add(definePattern("{x} >= {y}",             "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} >= {y}",             "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} >= {y}",             "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} >= {y}",             "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} >= {y}",             "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));	
	def_table->add(definePattern("{x} <= {y}",             "id,id",               comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} <= {y}",             "s,s",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} <= {y}",             "c,c",                 comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} <= {y}",             "c,is",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));
	def_table->add(definePattern("{x} <= {y}",             "is,c",                comp_desc,  SymbolID::Function,   Precedence(COMP_PREC),           Precedence(COMP_PREC)));

	def_table->add(definePattern("typeof {x}",              nullptr,              teval_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} as {y}",              "ids,ids",            tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} as {y}",              "i,c",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} as {y}",              "c,i",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} as {y}",              "c,s",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} like {y}",            "ids,ids",            tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} like {y}",            "i,c",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} like {y}",            "c,i",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} like {y}",            "c,s",                tcast_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} is {y}",              nullptr,              tcomp_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} is like {y}",         nullptr,              tcomp_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} isn't {y}",           nullptr,              tcomp_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));
	def_table->add(definePattern("{x} isn't like {y}",      nullptr,              tcomp_desc, SymbolID::Function,   Precedence(TLH_PREC),            Precedence(TRH_PREC)));

	def_table->add(definePattern("{x} and {y}",             "b,b",                op_desc,    SymbolID::Function,   Precedence(BOOL_PREC, AND_PREC), Precedence(BOOL_PREC, AND_PREC)));
	def_table->add(definePattern("{x} or {y}",              "b,b",                op_desc,    SymbolID::Function,   Precedence(BOOL_PREC, OR_PREC),  Precedence(BOOL_PREC, OR_PREC)));
	def_table->add(definePattern("not {x}",                 "b",                  op_desc,    SymbolID::Function,   Precedence(BOOL_PREC, NOT_PREC), Precedence(BOOL_PREC, NOT_PREC)));

	//def_table->add(definePattern("[ {x} , {y} ]",   number_restrictions,  range_desc, SymbolID::Function));
	//def_table->add(definePattern("[ {x} , {y} )",   number_restrictions,  range_desc, SymbolID::Function));
	//def_table->add(definePattern("( {x} , {y} ]",   number_restrictions,  range_desc, SymbolID::Function));
	//def_table->add(definePattern("( {x} , {y} )",   number_restrictions,  range_desc, SymbolID::Function));

	def_table->add(definePattern("output {x}",              "idbcs",              sys_desc,   SymbolID::Function,   Precedence(SYS_PREC),            Precedence(SYS_PREC)));
	def_table->add(definePattern("input",                   nullptr,              sys_desc,   SymbolID::Function));
	def_table->add(definePattern("start timer",             nullptr,              sys_desc,   SymbolID::Function));
	def_table->add(definePattern("timer value",             nullptr,              sys_desc,   SymbolID::Function));
	def_table->add(definePattern("random integer",          nullptr,              sys_desc,   SymbolID::Function));

	def_table->add(definePattern("{x} array",               nullptr,              arr_desc,   SymbolID::Function,   Precedence(ARR_PREC),            Precedence(ARR_PREC)));
	def_table->add(definePattern("{x} array of length {y}", "*,i",                arr_desc,   SymbolID::Function,   Precedence(ARR_PREC),            Precedence()));
	def_table->add(definePattern("{x} [ {y} ]",             "as,i",               arr_desc,   SymbolID::Function,   Precedence(ARR_PREC),            Precedence(ARR_PREC),              true));
	def_table->add(definePattern("length of {x}",           "as",                 arr_desc,   SymbolID::Function,   Precedence(ARR_PREC),            Precedence(ARR_PREC)));

	return def_table;

}

AST::ptr_Symbol AST::definePattern(const char* header, const char* restrictions, const char* description, SymbolID pattern_type, Precedence p_lh, Precedence p_rh, bool is_assignable) {

	static vec<string> arg_names;
	arg_names.clear();
	
	ptr_HeaderSymbol sym = new HeaderSymbol(pattern_type);

	string arg;
	string filler;

	bool is_filler = true;
	int i = 0;

	while (header[i]) {

		char c = header[i];

		if (isWhitespace(c));

		else if (c == '{') {

			if (filler.size() > 0) sym->add(SymbolComponent{ filler.c_str(), false });

			is_filler = false;
			filler.clear();

		}

		else if (c == '}') {

			if (arg.size() > 0) {

				sym->add(SymbolComponent{ arg.c_str(), true });
				arg_names.push_back(arg);

			}

			is_filler = true;
			arg.clear();

		}
		else {

			if (is_filler) filler += c;
			else arg += c;

		}

		i++;
		
	}

	if (filler.size() > 0) sym->add(SymbolComponent{ filler.c_str(), false });

	i = 0;
	int arg_index = 0;

	auto new_rest = new Restrictions();

	auto& r = *new_rest;
	sym->restrictions = new_rest;

	if (restrictions) while (restrictions[i]) {

		char c = restrictions[i];

		if (c == '*');

		else if (c == ',') arg_index++;

		else {
			
			auto& arg_name = arg_names[arg_index];
			auto& cur_vec = r[arg_index];

			if (c == 'i') cur_vec.push_back(Type::Integer);
			else if (c == 'd') cur_vec.push_back(Type::Decimal);
			else if (c == 'b') cur_vec.push_back(Type::Bool);
			else if (c == 'c') cur_vec.push_back(Type::Ascii);
			else if (c == 's') cur_vec.push_back(Type::String);
			else if (c == 'a') cur_vec.push_back(Type::Array);
			else if (c == 'n') cur_vec.push_back(Type::Nothing);

		}

		i++;

	}
	
	sym->precedence_lh = p_lh;
	sym->precedence_rh = p_rh;
	sym->description = description;
	sym->is_assignable = is_assignable;
	sym->restrictions_eval_progress = HeaderSymbol::RestrictionsEvaluationProgress::Complete;

	return sym.cast<Symbol>();

}
