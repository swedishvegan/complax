#include "./Declaration.hpp"

AST::Declaration::Declaration() : Pattern(PatternID::Declaration) { precedence = -1; }

string AST::Declaration::getInfo(int) { return sym->oneLineDescription().c_str(); }

AST::Scanner_Declaration::Scanner_Declaration(Code::Loader& loader, int start, int end, int cur_best_start) : Scanner(loader, start, end, cur_best_start) {

	Scanner_Declaration_base decl(loader, start, end, cur_best_start);

	if (decl.result == nullptr) return;

	ptr_Symbol sym = new Symbol(SymbolID::Variable);

	sym->add(SymbolComponent{ decl.result.cast<FreePattern>()->name, false });
	sym->scope = loader(start).scope;

	ptr_Declaration new_decl = new Declaration();

	new_decl->start = decl.result->start;
	new_decl->end = decl.result->end;
	new_decl->sym = sym;

	result = new_decl.cast<Pattern>();
	
}