#include "./SymbolTable.hpp"
#include "./SymbolTableLinker.hpp"

AST::SymbolTable::SymbolTable(int scope_size) : num_buckets((size_t)((double)scope_size * 1.5 + 1.0)) { }

AST::SymbolList& AST::SymbolTable::getSymbolList(SymbolID ID) {

	static SymbolList _dummy;

	if (ID == SymbolID::Function) return functions;
	else if (ID == SymbolID::Structure) return structures;
	else if (ID == SymbolID::Variable) return variables;

	return _dummy;

}

bool AST::SymbolTable::conflictsWith(Symbol& sym) {

	if (sym.ID == SymbolID::Function) { for (auto s : functions) if (sym == *s) return true; }
	else if (sym.ID == SymbolID::Structure) { for (auto s : structures) if (sym == *s) return true; }
	else if (sym.ID == SymbolID::Variable) for (auto s : variables) if (sym == *s) return true;

	return false;

}

void AST::SymbolTable::add(ptr_Symbol sym) {

	if (sym == nullptr) return;

	sym->index = this->getSymbolList(sym->ID).size();

	if (sym->ID == SymbolID::Function) functions.push_back(sym);
	else if (sym->ID == SymbolID::Structure) structures.push_back(sym);
	else if (sym->ID == SymbolID::Variable) variables.push_back(sym);

}

void AST::SymbolTable::include(ptr_SymbolTable table) {

	if (table == nullptr) return;

	for (auto inc : includes) if (table == inc) return;
	includes.push_back(table);

}

void AST::SymbolTable::include(SymbolTableLinker linker) {

	auto cur_linker = &linker;
	while (cur_linker) { include(cur_linker->table); cur_linker = cur_linker->next(); }

}

string AST::SymbolTable::toString(int alignment) {

	string str = "SymbolTable:\n";

	str += indent(alignment + 1) + string("Functions") + (functions.size() == 0 ? " (empty)\n" : ":\n");
	for (auto sym : functions) str += sym->print(alignment + 2);

	str += indent(alignment + 1) + string("Structures") + (functions.size() == 0 ? " (empty)\n" : ":\n");
	for (auto sym : structures) str += sym->print(alignment + 2);

	str += indent(alignment + 1) + string("Variables") + (variables.size() == 0 ? " (empty)\n" : ":\n");
	for (auto sym : variables) str += sym->print(alignment + 2);

	return str.substr(0, str.size() - 1);

}