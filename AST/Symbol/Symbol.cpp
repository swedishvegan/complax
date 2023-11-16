#include <sstream>
#include <iomanip>
#include "./Symbol.hpp"
#include "./../utility_functions.hpp"

const char* _SymbolID_strings[] = {

	"None",

	"Function",
	"Structure",
	"Variable",

};

AST::SymbolComponent::SymbolComponent() { }

AST::SymbolComponent::SymbolComponent(managed_string name, bool is_argument) : name(name), is_argument(is_argument) { }

bool AST::SymbolComponent::operator == (SymbolComponent& comp) { return is_argument == comp.is_argument && name == comp.name; }

bool AST::SymbolComponent::operator != (SymbolComponent& comp) { return !(*this == comp); }

string AST::SymbolComponent::toString(int) { return pad(is_argument ? "Argument:" : "Filler:", AST_SYMBOL_PAD_LENGTH) + string(name.c_str()); }

AST::Symbol::Symbol(SymbolID ID) : ID(ID) { }

int AST::Symbol::size() { return components.size(); }

AST::SymbolComponent& AST::Symbol::operator [] (int index) {

	if (index < 0) index = 0;
	if (index >= size()) index = size() - 1;

	return components[index];

}

void AST::Symbol::add(SymbolComponent comp) { 
	
	components.push_back(comp); 
	
	if (comp.is_argument) num_arguments++; 
	else num_fillers++;

}

bool AST::Symbol::operator == (Symbol& sym) {

	if (ID != sym.ID) return false;
	if (size() != sym.size()) return false;
	for (int i = 0; i < size(); i++) if ((*this)[i] != sym[i]) return false;

	return true;

}

bool AST::Symbol::operator != (Symbol& sym) { return !(*this == sym); }

managed_string& AST::Symbol::getSignature() {

	if (signature.size() == 0) signature = oneLineDescription(false);
	return signature;

}

string AST::Symbol::toString(int) {

	if (components.size() == 0) return _SymbolID_strings[(int)ID] + string(" (empty)");
	return pad(_SymbolID_strings[(int)ID] + string(":"), AST_SYMBOL_PAD_LENGTH) + oneLineDescription().c_str();

}

managed_string AST::Symbol::oneLineDescription(bool include_arg_names) {

	if (components.size() == 0) return "(empty)";

	managed_string s;

	for (int i = 0; i < components.size(); i++) {

		auto& comp = components[i];

		if (comp.is_argument) s += "{" + (include_arg_names ? comp.name : managed_string()) + "}"; 
		else s += comp.name;

		if (i < components.size() - 1) s += " ";

	}

	return s;

}

AST::Precedence::Precedence(double p0, double p1, double p2) : p0(p0), p1(p1), p2(p2) { }

bool AST::Precedence::operator >= (Precedence rh) { 
	
	if (p0 > rh.p0) return true;
	if (p0 < rh.p0) return false;

	if (p1 > rh.p1) return true;
	if (p1 < rh.p1) return false;

	return p2 >= rh.p2;

}

bool AST::Precedence::operator > (Precedence rh) { 

	if (p0 > rh.p0) return true;
	if (p0 < rh.p0) return false;

	if (p1 > rh.p1) return true;
	if (p1 < rh.p1) return false;

	return p2 > rh.p2;

}

bool AST::Precedence::operator <= (Precedence rh) { return rh >= *this; }

bool AST::Precedence::operator < (Precedence rh) { return rh > *this; }

bool AST::Precedence::operator == (Precedence rh) { return p0 == rh.p0 && p1 == rh.p1 && p2 == rh.p2; }

bool AST::Precedence::operator != (Precedence rh) { return !(rh == *this); }

string AST::Precedence::toString(int) { 
	
	std::stringstream stream;
	stream << std::fixed << std::setprecision(3) << "(" << p0 << ", " << p1 << ", " << p2 << ")";

	return stream.str();

}

AST::HeaderSymbol::HeaderSymbol(SymbolID ID) : Symbol(ID) { }

void AST::HeaderSymbol::add(SymbolComponent comp) {

	int i = components.size();

	if (comp.is_argument) { argument_indices.push_back(i); num_arguments++; }
	else { filler_indices.push_back(i); num_fillers++; }

	components.push_back(comp);

}

int AST::HeaderSymbol::distanceToNextFiller(int filler_index) {
	
	int cur_filler = filler_indices[filler_index];
	int next_filler = (filler_index == num_fillers - 1) ? size() - 1 : filler_indices[filler_index + 1];

	return next_filler - cur_filler;

}