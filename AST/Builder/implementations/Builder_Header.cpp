#include "./Builder_Header.hpp"
#include "./implement_builder.hpp"
#include "./../../Pattern/implementations/FreePattern.hpp"

AST::Builder_Header::Builder_Header(Code::Loader& loader, int start, int end, int scope, SymbolID symbol_type) 
	: Builder(symbol_type == SymbolID::Function ? BuilderID::Builder_Header_Function : BuilderID::Builder_Header_Structure, loader, start, end), sym(new HeaderSymbol(symbol_type)) 
	{ table->is_header = true; }

AST_DECL_SUCCESSORS(Builder_Header) {

	if (AST_SUCC_ISTYPE(None) || AST_SUCC_ISTYPE(Argument)) {

		AST_SUCC_ALLOW(Argument);
		AST_SUCC_ALLOW(Filler);

	}

	AST_SUCC_CASE(Filler) AST_SUCC_ALLOW(Argument);

	if (!AST_SUCC_ISTYPE(None)) AST_SUCC_ALLOW(Done);

}

bool AST::Builder_Header::processPattern(ptr_Pattern p) {

	bool is_filler = p->ID == PatternID::Filler;
	bool is_argument = p->ID == PatternID::Argument;

	if (!is_filler && !is_argument) return true;

	managed_string name = p.cast<FreePattern>()->name;

	if (is_argument) {

		ptr_Symbol arg = new Symbol(SymbolID::Variable);

		arg->add(SymbolComponent{ name, false });
		arg->scope = main_scope;
		arg->is_argument = true;
		
		if (table->conflictsWith(*arg)) {

			error.error = true;
			error.info = "Argument conflict in header.";
			error.sources.push_back(p.cast<Printable>());
			
			finished = true;
			return false;

		}

		table->add(arg);

	}

	sym->add(SymbolComponent{ name, is_argument });
	return true;

}

void AST::Builder_Header::noFillersError() {

	if (sym->num_fillers > 0) return;

	error.error = true;
	error.info = "Pattern match definition has no fillers.";

	this->single_line = true;

	error.sources.push_back(new PrintableString(this->print(0, false)));

}

AST::Builder_Header_Function::Builder_Header_Function(Code::Loader& loader, int start, int end, int scope) : Builder_Header(loader, start, end, scope, SymbolID::Function) { 
	
	build();
	noFillersError();
	
}

AST::Builder_Header_Structure::Builder_Header_Structure(Code::Loader& loader, int start, int end, int scope) : Builder_Header(loader, start, end, scope, SymbolID::Structure) { 
	
	build(); 
	noFillersError();

}