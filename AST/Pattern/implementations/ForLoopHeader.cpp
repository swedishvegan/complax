#include "./ForLoopHeader.hpp"

AST::ForLoopHeader::ForLoopHeader() : Pattern(PatternID::ForLoopHeader) { }

string AST::ForLoopHeader::getInfo(int alignment) {
	
	if (iterator == nullptr) return "";

	string s = "\n";

	s += indent(alignment + 1) + "Variable:\n";
	s += indent(alignment + 2) + table->variables[0]->oneLineDescription().c_str() + "\n";
	s += iterator->print(alignment + 1, false);

	return s;

}

AST::Scanner_ForLoopHeader::Scanner_ForLoopHeader(Code::Loader& loader, int start, int end, int cur_best_start, SymbolTableLinker symbols) : Scanner(loader, start, end, cur_best_start), loader(loader), start(start), end(end), symbols(symbols) {

	int scan_start = start;
	while (loader(scan_start).is_whitespace) scan_start++;

	int orig_scan_start = scan_start;

	scan_start++; if (scan_start >= end) return;

	ptr_Expression iterator;

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::None);

	while (true) {

		auto candidate = findIterator(scan_start);

		if (candidate != nullptr) { iterator = candidate; break; }
		else if (scan_start < end) scan_start++;
		else return;

	}

	result = getHeader(orig_scan_start, scan_start, iterator).cast<Pattern>();

}

AST::ptr_Expression AST::Scanner_ForLoopHeader::findIterator(int scan_start) {
	
	int iterator_start = matchString(loader, "in", scan_start);
	if (iterator_start < 0) return nullptr;
	
	Scanner_Expression scanner(loader, iterator_start, end, 0, symbols);
	return (scanner.result == nullptr) ? nullptr : scanner.result.cast<Expression>();

}

AST::ptr_ForLoopHeader AST::Scanner_ForLoopHeader::getHeader(int orig_scan_start, int scan_start, ptr_Expression iterator) {

	managed_string varname;
	for (int i = orig_scan_start; i < scan_start; i++) if (!loader(i).is_whitespace) varname += loader[i];

	ptr_Symbol var_sym = new Symbol(SymbolID::Variable);

	var_sym->add(SymbolComponent{ varname, false });
	var_sym->scope = loader(start).scope;

	ptr_SymbolTable table = new SymbolTable(1);
	table->add(var_sym);

	ptr_ForLoopHeader flh = new ForLoopHeader();

	flh->start = loader(start);
	flh->end = iterator->end;

	flh->table = table;
	flh->iterator = iterator;

	if (flh->iterator->error.error) flh->error = flh->iterator->error;

	return flh;

}